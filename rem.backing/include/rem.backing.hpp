#pragma once

#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/eosio.hpp>

namespace eosio {

   class [[eosio::contract("rem.backing")]] backing : public contract {
   public:

      backing(name receiver, name code,  datastream<const char*> ds);

      /**
       * Distribute rewards action.
       *
       * @details distribute rewards in proportion between the accounts indicated in the table.
       */
      [[eosio::action]]
      void distrewards();

      /**
       * Set the accounts to which rewards will be distributed.
       *
       * @param accounts - the accounts to which rewards will be distributed,
       * @param reward_pct - the account for which the token balance is returned.
       */
      [[eosio::action]]
      void setaccounts(const std::vector<name> &accounts, const std::vector<double> &reward_pct);

      /**
       * Remove account from the rewards distribution list.
       *
       * @param account - the account to be deleted.
       */
      [[eosio::action]]
      void removeacc(const name &account);

      /**
       * Get balance method.
       *
       * @details Get the balance for a token `sym_code` created by `token_contract_account` account,
       * for account `owner`.
       *
       * @param token_contract_account - the token creator account,
       * @param owner - the account for which the token balance is returned,
       * @param sym_code - the token for which the balance is returned.
       */
      static asset get_balance( const name& token_contract_account, const name& owner, const symbol& sym )
      {
          accounts accountstable( token_contract_account, owner.value );
          const auto& ac = accountstable.get( sym.code().raw(), "balance not found" );
          return ac.balance;
      }

      using create_action = action_wrapper<"distrewards"_n, &backing::distrewards>;
   private:
      static constexpr name   system_account = "rem"_n;
      static constexpr name   token_account  = "rem.token"_n;
      static constexpr symbol core_symbol    = symbol(symbol_code("REM"), 4);

      struct [[eosio::table]] rewardsdata {
         std::map<name, double> reward_distribution;

         // explicit serialization macro is not necessary, used here only to improve compilation time
         EOSLIB_SERIALIZE( rewardsdata, (reward_distribution))
      };

      struct [[eosio::table]] account {
         asset    balance;

         // explicit serialization macro is not necessary, used here only to improve compilation time
         uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };

      typedef eosio::multi_index< "accounts"_n,  account >  accounts;
      typedef singleton< "rewarddist"_n,  rewardsdata >    rewards_idx;

      rewards_idx  rewards_dist_tbl;
      rewardsdata rewards_dist;

      void claim_rewards(const name &owner);
      void delegatebw(const name& from, const name& receiver, const asset& stake_quantity, bool transfer);
   };
   /** @}*/ // end of @defgroup rem.reward
} /// namespace eosio
