#pragma once

#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/eosio.hpp>

namespace eosio {

   class [[eosio::contract("guardian.reward")]] reward : public contract {
   public:

      reward(name receiver, name code,  datastream<const char*> ds);

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
      [[eosio::action]]
      void distribute();

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
      [[eosio::action]]
      void setaccounts(const std::vector<name> &guardian, const std::vector<double> &reward_pct);

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
      [[eosio::action]]
      void removeacc(const name &guardian);

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

      using create_action = action_wrapper<"distribute"_n, &reward::distribute>;
   private:
      static constexpr name system_account = "rem"_n;
      static constexpr name token_account  = "rem.token"_n;
      static constexpr symbol core_symbol  = symbol(symbol_code("REM"), 4);

      struct [[eosio::table]] guardians_data {
         std::map<name, double> reward_distribution;

         // explicit serialization macro is not necessary, used here only to improve compilation time
         EOSLIB_SERIALIZE( guardians_data, (reward_distribution))
      };

      struct [[eosio::table]] account {
         asset    balance;

         uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };

      typedef eosio::multi_index< "accounts"_n,  account >  accounts;
      typedef singleton< "rewarddist"_n,  guardians_data >  guardians_idx;

      guardians_idx guardians_tbl;
      guardians_data guardians;

      void claim_rewards(const name &owner);
      void transfer_tokens(const name &from, const name &to, const asset &quantity, const std::string &memo);
   };
   /** @}*/ // end of @defgroup rem.reward
} /// namespace eosio
