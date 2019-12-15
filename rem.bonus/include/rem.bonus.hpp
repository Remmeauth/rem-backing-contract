#pragma once

#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/eosio.hpp>

#include <numeric>

namespace eosio {

   class [[eosio::contract("rem.bonus")]] bonus : public contract {
   public:

      bonus(name receiver, name code,  datastream<const char*> ds);

      /**
       * Distribute rewards action.
       *
       * @details distribute rewards in proportion between the accounts indicated in the table.
       */
      [[eosio::action]]
      void distrewards();

      /**
       * Add the accounts to which rewards will be distributed.
       *
       * @param accounts - the accounts to which rewards will be distributed,
       * @param reward_index - the reward index of a receiving account.
       */
      [[eosio::action]]
      void addaccounts(const std::vector<name> &accounts, const std::vector<double> &reward_index);

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

      using create_action = action_wrapper<"distrewards"_n, &bonus::distrewards>;
   private:
      static constexpr name     system_account       = "rem"_n;
      static constexpr name     token_account        = "rem.token"_n;
      static constexpr symbol   core_symbol          = symbol(symbol_code("REM"), 4);
      static constexpr uint32_t min_contract_balance = 1'0000;

      struct [[eosio::table, eosio::contract("rem.bonus")]] rewards {
         std::map<name, double> distribution;

         // explicit serialization macro is not necessary, used here only to improve compilation time
         EOSLIB_SERIALIZE( rewards, (distribution))
      };

      struct [[eosio::table]] account {
         asset    balance;

         // explicit serialization macro is not necessary, used here only to improve compilation time
         uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };

      /**
       * Voter info.
       *
       * @details Voter info stores information about the voter:
       * - `owner` the voter
       * - `proxy` the proxy set by the voter, if any
       * - `producers` the producers approved by this voter if no proxy set
       * - `staked` the amount staked
       */
      struct [[eosio::table, eosio::contract("rem.system")]] voter_info {
       public:
         name                owner;     /// the voter
         name                proxy;     /// the proxy set by the voter, if any
         std::vector<name>   producers; /// the producers approved by this voter if no proxy set
         int64_t             staked = 0;
         int64_t             locked_stake = 0;

         double by_stake() const { return staked; }

         /**
          *  Every time a vote is cast we must first "undo" the last vote weight, before casting the
          *  new vote weight.  Vote weight is calculated as:
          *
          *  stated.amount * 2 ^ ( weeks_since_launch/weeks_per_year)
          */
         double              last_vote_weight = 0; /// the vote weight cast the last time the vote was updated
         time_point          stake_lock_time;
         time_point          last_undelegate_time;

         /**
          * Total vote weight delegated to this voter.
          */
         double              proxied_vote_weight= 0; /// the total vote weight delegated to this voter as a proxy
         bool                is_proxy = 0; /// whether the voter is a proxy for others


         uint32_t            flags1 = 0;
         uint32_t            reserved2 = 0;
         eosio::asset        reserved3;

         uint64_t primary_key()const { return owner.value; }

         enum class flags1_fields : uint32_t {
            ram_managed = 1,
            net_managed = 2,
            cpu_managed = 4
         };

         time_point          last_reassertion_time;
         int64_t             pending_perstake_reward = 0;
         time_point          last_claim_time;


         // explicit serialization macro is not necessary, used here only to improve compilation time
         EOSLIB_SERIALIZE( voter_info, (owner)(proxy)(producers)(staked)(locked_stake)(last_vote_weight)
            (stake_lock_time)(last_undelegate_time)(proxied_vote_weight)(is_proxy)(flags1)(reserved2)(reserved3)
            (last_reassertion_time)(pending_perstake_reward)(last_claim_time) )
      };

      /**
       * Defines new global state parameters to store remme specific parameters
       */
      struct [[eosio::table("globalrem"), eosio::contract("rem.system")]] eosio_global_rem_state {
         double  per_stake_share = 0.6;
         double  per_vote_share  = 0.3;

         name gifter_attr_contract = name{"rem.attr"};
         name gifter_attr_issuer   = name{"rem.attr"};
         name gifter_attr_name     = name{"accgifter"};

         int64_t guardian_stake_threshold = 250'000'0000LL;
         microseconds producer_max_inactivity_time = eosio::minutes(30);
         microseconds producer_inactivity_punishment_period = eosio::days(30);

         microseconds stake_lock_period   = eosio::days(180);
         microseconds stake_unlock_period = eosio::days(180);

         microseconds reassertion_period = eosio::days( 7 );

         EOSLIB_SERIALIZE( eosio_global_rem_state, (per_stake_share)(per_vote_share)
            (gifter_attr_contract)(gifter_attr_issuer)(gifter_attr_name)
            (guardian_stake_threshold)(producer_max_inactivity_time)(producer_inactivity_punishment_period)
            (stake_lock_period)(stake_unlock_period)(reassertion_period) )
      };

      typedef eosio::multi_index< "accounts"_n,  account >  accounts;
      typedef singleton< "rewards"_n,  rewards >            rewards_idx;
      /**
       * Voters table
       *
       * @details The voters table stores all the `voter_info`s instances, all voters information.
       */
      typedef eosio::multi_index< "voters"_n, voter_info,
                                  indexed_by<"bystake"_n, const_mem_fun<voter_info, double, &voter_info::by_stake> >
                                > voters_table;
      typedef eosio::singleton< "globalrem"_n, eosio_global_rem_state > global_rem_state_singleton;

      rewards_idx                 rewards_tbl;
      rewards                     rewards_info;
      voters_table                voters;
      global_rem_state_singleton  global;
      eosio_global_rem_state      global_data;

      double get_total_accounts_stake();

      void check_share_sum(const double &total_accounts_stake);
      bool is_guardian(const name &account);

      void claim_rewards(const name &owner);
      void delegatebw(const name& from, const name& receiver, const asset& stake_quantity, bool transfer);
   };
   /** @}*/ // end of @defgroup rem.reward
} /// namespace eosio
