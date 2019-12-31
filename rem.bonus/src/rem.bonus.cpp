#include <rem.bonus.hpp>

namespace eosio {

   void bonus::distrewards()
   {
      asset contract_balance = get_balance(token_account, get_self(), core_symbol);
      check(contract_balance.amount >= min_contract_balance, "the balance of the contract should contain a minimum amount for distribution");

      double total_accounts_stake = get_total_accounts_stake();
      const uint8_t max_dist_depth = 10;
      size_t i = 0;
      for (auto table_it = rewards_tbl.begin(); table_it != rewards_tbl.end(); ++table_it) {
         if (table_it->is_distribute_today) continue;
         if (i >= max_dist_depth) break;
         const auto vit = voters.find(table_it->account.value);
         int64_t account_stake = vit != voters.end() ? vit->staked : 0;

         double account_share = (account_stake * table_it->reward_index) / total_accounts_stake;
         asset quantity = { static_cast<int64_t>(contract_balance.amount * account_share), core_symbol };
         if (quantity.amount >= 1 && is_guardian(table_it->account)) {    // 0.0001 REM
            delegatebw(get_self(), table_it->account, quantity, true);
         }

         rewards_tbl.modify(*table_it, same_payer, [&](auto &s) {
            s.is_distribute_today = true;
         });
         ++i;
      }
   }

   void bonus::addaccounts(const std::vector<name> &accounts, const std::vector<double> &reward_index)
   {
      require_auth( get_self() );

      for (size_t i = 0; i < accounts.size(); ++i) {
         check(reward_index.at(i) >= 1 && reward_index.at(i) <= 1.5, "reward index should be between 1 and 1.5");
         auto it = rewards_tbl.find(accounts.at(i).value);
         if (it == rewards_tbl.end()) {
            rewards_tbl.emplace(get_self(), [&](auto &s) {
               s.account = accounts.at(i);
               s.reward_index = reward_index.at(i);
               s.is_distribute_today = false;
            });
         } else {
            rewards_tbl.modify(*it, get_self(), [&](auto &s) {
               s.reward_index = reward_index.at(i);
            });
         }
      }
   }

   void bonus::removeacc(const name &account)
   {
      require_auth( get_self() );
      auto it = rewards_tbl.find(account.value);
      check(it != rewards_tbl.end(), "account not found");

      rewards_tbl.erase(it);
   }

   void bonus::claimrewards()
   {
      for (auto table_it = rewards_tbl.begin(); table_it != rewards_tbl.end(); ++table_it) {
         check(table_it->is_distribute_today, "rewards are not distributed to all accounts");
         rewards_tbl.modify(*table_it, same_payer, [&](auto &s) {
            s.is_distribute_today = false;
         });
      }

      action(
         permission_level{get_self(), "active"_n},
         system_account, "claimrewards"_n,
         std::make_tuple(get_self())
      ).send();
   }

   double bonus::get_total_accounts_stake() {
      double total_accounts_stake = 0;
      for (auto table_it = rewards_tbl.begin(); table_it != rewards_tbl.end(); ++table_it) {
         if (!table_it->is_distribute_today && is_guardian(table_it->account)) {
            const auto vit = voters.find(table_it->account.value);
            int64_t account_stake = vit != voters.end() ? vit->staked : 0;

            total_accounts_stake += account_stake * table_it->reward_index;
         }
      }
      return total_accounts_stake;
   }

   bool bonus::is_guardian(const name &account) {
      const auto vit = voters.find(account.value);
      eosio_global_rem_state global_data = global.get();
      int64_t account_stake = vit != voters.end() ? vit->staked : 0;
      const auto ct = current_time_point();

      if (account_stake >= global_data.guardian_stake_threshold && ct - vit->last_reassertion_time <= global_data.reassertion_period) {
         return true;
      }
      return false;
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

EOSIO_DISPATCH( eosio::bonus, (distrewards)(claimrewards)(addaccounts)(removeacc) )
