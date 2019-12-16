#include <rem.bonus.hpp>

namespace eosio {

   bonus::bonus(name receiver, name code,  datastream<const char*> ds)
   :contract(receiver, code, ds),
    rewards_tbl(get_self(), get_self().value),
    voters(system_account, system_account.value),
    global(system_account, system_account.value)
    {
      rewards_info = rewards_tbl.exists() ? rewards_tbl.get() : rewards{};
      global_data = global.get();
    }

   void bonus::distrewards()
   {
      claim_rewards( get_self() );
      asset contract_balance = get_balance(token_account, get_self(), core_symbol);
      check(contract_balance.amount >= min_contract_balance, "the balance of the contract should contain a minimum amount for distribution");

      double total_accounts_stake = get_total_accounts_stake();

      for (const auto &account: rewards_info.distribution) {
         const auto vit = voters.find(account.first.value);
         int64_t account_stake = vit != voters.end() ? vit->staked : 0;

         double account_share = (account_stake * account.second) / total_accounts_stake;
         asset quantity = { static_cast<int64_t>(contract_balance.amount * account_share), core_symbol };
         if (is_guardian(account.first) && quantity.amount >= 1) {    // 0.0001 REM
            delegatebw(get_self(), account.first, quantity, true);
         }
      }
   }

   void bonus::addaccounts(const std::vector<name> &accounts, const std::vector<double> &reward_index)
   {
      require_auth( get_self() );

      for (size_t i = 0; i < accounts.size(); ++i) {
         check(reward_index.at(i) >= 1 && reward_index.at(i) <= 1.5, "reward index should be between 1 and 1.5");
         rewards_info.distribution[accounts.at(i)] = reward_index.at(i);
      }
      rewards_tbl.set(rewards_info, get_self());
   }

   void bonus::removeacc(const name &account)
   {
      require_auth( get_self() );

      auto it = rewards_info.distribution.find(account);
      check(it != rewards_info.distribution.end(), "account not found");

      rewards_info.distribution.erase(account);
      rewards_tbl.set(rewards_info, same_payer);
   }

   double bonus::get_total_accounts_stake() {
      double total_accounts_stake = 0;
      for (const auto &account: rewards_info.distribution) {
         const auto vit = voters.find(account.first.value);
         int64_t account_stake = vit != voters.end() ? vit->staked : 0;
         if (is_guardian(account.first))
            total_accounts_stake += account_stake * account.second;
      }
      return total_accounts_stake;
   }

   bool bonus::is_guardian(const name &account) {
      const auto vit = voters.find(account.value);
      int64_t account_stake = vit != voters.end() ? vit->staked : 0;
      const auto ct = current_time_point();

      if (account_stake >= global_data.guardian_stake_threshold && ct - vit->last_reassertion_time <= global_data.reassertion_period) {
         return true;
      }
      return false;
   }

   void bonus::claim_rewards(const name &owner)
   {
      action(
         permission_level{owner, "active"_n},
         system_account, "claimrewards"_n,
         std::make_tuple(owner)
      ).send();
   }

   void bonus::delegatebw(const name& from, const name& receiver, const asset& stake_quantity, bool transfer)
   {
      action(
         permission_level{from, "active"_n},
         system_account, "delegatebw"_n,
         std::make_tuple(from, receiver, stake_quantity, transfer)
      ).send();
   }
} /// namespace eosio

EOSIO_DISPATCH( eosio::bonus, (distrewards)(addaccounts)(removeacc) )
