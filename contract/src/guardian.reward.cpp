#include <guardian.reward.hpp>

namespace eosio {

   reward::reward(name receiver, name code,  datastream<const char*> ds)
   :contract(receiver, code, ds),
    guardians_tbl(get_self(), get_self().value)
    {
      guardians = guardians_tbl.exists() ? guardians_tbl.get() : guardians_data{};
    }

   void reward::distribute()
   {
      claim_rewards( get_self() );

      asset contract_balance = get_balance(token_account, get_self(), core_symbol);
      check(contract_balance.amount > 0, "contract balance should be a positive");

      for (const auto &guardian: guardians.reward_distribution) {
         asset quantity = { static_cast<int64_t>(contract_balance.amount * guardian.second), core_symbol };
         if (quantity.amount > 0) {
            transfer_tokens(get_self(), guardian.first, quantity, "remme distribution reward");
         }
      }
   }

   void reward::setaccounts(const std::vector<name> &guardian, const std::vector<double> &reward_pct)
   {
      require_auth( get_self() );

      for (size_t i = 0; i < guardian.size(); ++i) {
         guardians.reward_distribution[guardian.at(i)] = reward_pct.at(i);
      }
   guardians_tbl.set(guardians, same_payer);
   }

   void reward::removeacc(const name &guardian)
   {
      require_auth( get_self() );

      auto it = guardians.reward_distribution.find(guardian);
      check(it != guardians.reward_distribution.end(), "guardian not found");

      guardians.reward_distribution.erase(guardian);
      guardians_tbl.set(guardians, same_payer);
   }

   void reward::claim_rewards(const name &owner)
   {
      action(
         permission_level{owner, "active"_n},
         "rem"_n, "claimrewards"_n,
         std::make_tuple(owner)
      ).send();
   }

   void reward::transfer_tokens(const name &from, const name &to, const asset &quantity, const std::string &memo)
   {
      action(
         permission_level{from, "active"_n},
         token_account, "transfer"_n,
         std::make_tuple(from, to, memo)
      ).send();
   }
} /// namespace eosio

EOSIO_DISPATCH( eosio::reward, (distribute)(setaccounts)(removeacc) )

