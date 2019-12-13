#include <rem.backing.hpp>

namespace eosio {

   backing::backing(name receiver, name code,  datastream<const char*> ds)
   :contract(receiver, code, ds),
    rewards_dist_tbl(get_self(), get_self().value),
    voters(system_account, system_account.value),
    global(system_account, system_account.value)
    {
      rewards_dist = rewards_dist_tbl.exists() ? rewards_dist_tbl.get() : rewarddist{};
      global_data = global.get();
    }

   void backing::distrewards()
   {
      claim_rewards( get_self() );
      asset contract_balance = get_balance(token_account, get_self(), core_symbol);
      check(contract_balance.amount >= 1'0000, "contract balance should be at least 1.0000 REM");
      double total_accounts_stake = get_total_accounts_stake();
      check_share_sum(total_accounts_stake);

      for (const auto &account: rewards_dist.reward_index) {
         asset account_balance = get_balance(token_account, account.first, core_symbol);
         bool is_guardian = check_is_guardian(account.first);

         if (is_guardian) {
            double account_share = (account_balance.amount * account.second) / total_accounts_stake;
            asset quantity = {static_cast<int64_t>(account_share * 1'0000), core_symbol };
            if (quantity.amount >= 1) {    // 0.0001 REM
               delegatebw(get_self(), account.first, quantity, true);
            }
         } else {
            total_accounts_stake -= account_balance.amount * account.second;
         }
      }
   }

   void backing::setaccounts(const std::vector<name> &accounts, const std::vector<double> &reward_pct)
   {
      require_auth( get_self() );

      for (size_t i = 0; i < accounts.size(); ++i) {
         rewards_dist.reward_index[accounts.at(i)] = reward_pct.at(i);
      }
      double total_accounts_stake = get_total_accounts_stake();
      check_share_sum(total_accounts_stake);
      rewards_dist_tbl.set(rewards_dist, get_self());
   }

   void backing::removeacc(const name &account)
   {
      require_auth( get_self() );

      auto it = rewards_dist.reward_index.find(account);
      check(it != rewards_dist.reward_index.end(), "account not found");

      rewards_dist.reward_index.erase(account);
      rewards_dist_tbl.set(rewards_dist, same_payer);
   }

   double backing::get_total_accounts_stake() {
      double total_accounts_stake = 0;
      for (const auto &account: rewards_dist.reward_index) {
         asset account_balance = get_balance(token_account, account.first, core_symbol);
         total_accounts_stake += account_balance.amount * account.second;
      }
      return total_accounts_stake;
   }

   void backing::check_share_sum(const double &total_accounts_stake) {
      double share = 0;
      for (const auto &account: rewards_dist.reward_index) {
         asset account_balance = get_balance(token_account, account.first, core_symbol);
         share += (account_balance.amount * account.second) / total_accounts_stake;
      }
      check(share >= 0.9999 && share <= 1.0001, "the sum of the share proportion should be a 1(+-0.0001");
   }

   bool backing::check_is_guardian(const name &account) {
      const auto vit = voters.find(account.value);
      const auto ct = current_time_point();

      if (vit != voters.end() && vit->staked >= global_data.guardian_stake_threshold && ct - vit->last_reassertion_time <= global_data.producer_inactivity_punishment_period) {
         return true;
      }
      return false;
   }

   void backing::claim_rewards(const name &owner)
   {
      action(
         permission_level{owner, "active"_n},
         system_account, "claimrewards"_n,
         std::make_tuple(owner)
      ).send();
   }

   void backing::delegatebw(const name& from, const name& receiver, const asset& stake_quantity, bool transfer)
   {
      action(
         permission_level{from, "active"_n},
         system_account, "delegatebw"_n,
         std::make_tuple(from, receiver, stake_quantity, transfer)
      ).send();
   }
} /// namespace eosio

EOSIO_DISPATCH( eosio::backing, (distrewards)(setaccounts)(removeacc) )
