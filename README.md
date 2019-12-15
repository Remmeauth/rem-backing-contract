# rem-backing-contract
The rem.backing public contract, which claims own reward and distributes it among the specified accounts.

[![Build Status](https://travis-ci.com/Remmeauth/rem-backing-contract.svg?branch=develop)](https://travis-ci.com/Remmeauth/rem-backing-contract)

  * [Actions](#Actions)
    * [distrewards](#distrewards)
    * [setaccounts](#setaccounts)
    * [removeacc](#removeacc)
  * [Building](#Building)
    * [Requirements](#Requirements)
  * [Deployment](#Deployment)

## Actions

### distrewards

Distribute rewards in proportion between the accounts indicated in the table.
Can be called once a day.
    
```bash
$ remcli push action rembacking distrewards '[]' -p accountnum1
```

### addaccounts

Add the accounts to which rewards will be distributed.
It can only be called by the owner of the contract.

```bash
$ remcli push action rembacking addaccounts '[["accountnum1", "accountnum2"], ["0.5", "0.5"]]' -p rembacking 
```

### removeacc

Remove account from the rewards distribution list.
It can only be called by the owner of the contract.

```bash
$ remcli push action rembacking removeacc '["accountnum1"]' -p rembacking
```

## Building

### Requirements

- Eosio.cdt v1.7 - https://github.com/eosio/eosio.cdt/releases/download/v1.7.0-rc1/
- Boost v1.67 or later - https://www.boost.org/

```bash
$ git clone https://github.com/Remmeauth/rem-backing-contract.git && cd rem-backing-contract
$ bash build.sh
```

Or, if you already worked with the project:

```bash
$ cd build/rem.backing && make
```

## Deployment

```bash
$ remcli set contract rembacking 'absolute_path'/remme-guardian-reward-contract/build/rem.backing --abi rem.backing.abi -p rembacking
```
