#include <rem.backing.hpp>

namespace eosio {

   backing::backing(name receiver, name code,  datastream<const char*> ds)
   :contract(receiver, code, ds),
    rewards_dist_tbl(get_self(), get_self().value)
    {
      rewards_dist = rewards_dist_tbl.exists() ? rewards_dist_tbl.get() : rewardsdata{};
    }

   void backing::distrewards()
   {
      claim_rewards( get_self() );
      asset contract_balance = get_balance(token_account, get_self(), core_symbol);

      for (const auto &guardian: rewards_dist.reward_distribution) {
         asset quantity = { static_cast<int64_t>(contract_balance.amount * guardian.second), core_symbol };
         if (quantity.amount > 0) {
            delegatebw(get_self(), guardian.first, quantity, true);
         }
      }
   }

   void backing::setaccounts(const std::vector<name> &accounts, const std::vector<double> &reward_pct)
   {
      require_auth( get_self() );

      for (size_t i = 0; i < accounts.size(); ++i) {
         rewards_dist.reward_distribution[accounts.at(i)] = reward_pct.at(i);
      }
   rewards_dist_tbl.set(rewards_dist, same_payer);
   }

   void backing::removeacc(const name &account)
   {
      require_auth( get_self() );

      auto it = rewards_dist.reward_distribution.find(account);
      check(it != rewards_dist.reward_distribution.end(), "account not found");

      rewards_dist.reward_distribution.erase(account);
      rewards_dist_tbl.set(rewards_dist, same_payer);
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
