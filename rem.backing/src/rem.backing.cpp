#include <rem.backing.hpp>

namespace eosio {

   backing::backing(name receiver, name code,  datastream<const char*> ds)
   :contract(receiver, code, ds),
    rewards_tbl(get_self(), get_self().value),
    voters(system_account, system_account.value),
    global(system_account, system_account.value)
    {
      rewards_info = rewards_tbl.exists() ? rewards_tbl.get() : rewards{};
      global_data = global.get();
    }

   void backing::distrewards()
   {
      claim_rewards( get_self() );
      asset contract_balance = get_balance(token_account, get_self(), core_symbol);
      check(contract_balance.amount >= min_contract_balance, "the balance of the contract should contain a minimum amount for distribution");

      double total_accounts_stake = get_total_accounts_stake();
      check_share_sum(total_accounts_stake);

      for (const auto &account: rewards_info.distribution) {
         const auto vit = voters.find(account.first.value);
         int64_t account_stake = vit != voters.end() ? vit->staked : 0;

         double account_share = (account_stake * account.second) / total_accounts_stake;
         print(contract_balance.amount * account_share);
         asset quantity = { static_cast<int64_t>(contract_balance.amount * account_share), core_symbol };
         if (is_guardian(account.first) && quantity.amount >= 1) {    // 0.0001 REM
            delegatebw(get_self(), account.first, quantity, true);
         }
      }
   }

   void backing::addaccounts(const std::vector<name> &accounts, const std::vector<double> &reward_index)
   {
      require_auth( get_self() );

      for (size_t i = 0; i < accounts.size(); ++i) {
         rewards_info.distribution[accounts.at(i)] = reward_index.at(i);
      }
      double total_accounts_stake = get_total_accounts_stake();
      check_share_sum(total_accounts_stake);
      rewards_tbl.set(rewards_info, get_self());
   }

   void backing::removeacc(const name &account)
   {
      require_auth( get_self() );

      auto it = rewards_info.distribution.find(account);
      check(it != rewards_info.distribution.end(), "account not found");

      rewards_info.distribution.erase(account);
      rewards_tbl.set(rewards_info, same_payer);
   }

   double backing::get_total_accounts_stake() {
      double total_accounts_stake = 0;
      for (const auto &account: rewards_info.distribution) {
         const auto vit = voters.find(account.first.value);
         int64_t account_stake = vit != voters.end() ? vit->staked : 0;
         if (is_guardian(account.first))
            total_accounts_stake += account_stake * account.second;
      }
      return total_accounts_stake;
   }

   void backing::check_share_sum(const double &total_accounts_stake) {
      double share = 0;
      for (const auto &account: rewards_info.distribution) {
         const auto vit = voters.find(account.first.value);
         int64_t account_stake = vit != voters.end() ? vit->staked : 0;
         if (is_guardian(account.first))
            share += (account_stake * account.second) / total_accounts_stake;
      }
      check(share >= 0.9999 && share <= 1.0001, "the sum of the share proportion should be a 1(+-0.0001");
   }

   bool backing::is_guardian(const name &account) {
      const auto vit = voters.find(account.value);
      int64_t account_stake = vit != voters.end() ? vit->staked : 0;
      const auto ct = current_time_point();

      if (account_stake >= global_data.guardian_stake_threshold && ct - vit->last_reassertion_time <= global_data.producer_inactivity_punishment_period) {
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

EOSIO_DISPATCH( eosio::backing, (distrewards)(addaccounts)(removeacc) )
