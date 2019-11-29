#pragma once

#include <eosio/eosio.hpp>

namespace eosio {

    class [[eosio::contract("rem.reward")]] reward : public contract {
    public:

        using contract::contract;

        [[eosio::action]]
        void distribute();

    private:
    };
    /** @}*/ // end of @defgroup eosioauth rem.oracle
} /// namespace eosio
