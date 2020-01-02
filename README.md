# rem-bonus-contract
The rem.bonus public contract, which claims own reward and distributes it among the specified accounts.

[![Build Status](https://travis-ci.com/Remmeauth/rem-bonus-contract.svg?branch=develop)](https://travis-ci.com/Remmeauth/rem-bonus-contract)

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
$ remcli push action rembonus distrewards '[]' -p accountnum1
```

### addaccounts

Add the accounts to which rewards will be distributed.
It can only be called by the owner of the contract.

```bash
$ remcli push action rembonus addaccounts '[["accountnum1", "accountnum2"], ["0.5", "0.5"]]' -p rembonus 
```

### removeacc

Remove account from the rewards distribution list.
It can only be called by the owner of the contract.

```bash
$ remcli push action rembonus removeacc '["accountnum1"]' -p rembonus
```

## Building

### Requirements

- Eosio.cdt v1.7 - https://github.com/eosio/eosio.cdt/releases/download/v1.7.0-rc1/
- Boost v1.67 or later - https://www.boost.org/

```bash
$ git clone https://github.com/Remmeauth/rem-bonus-contract.git && cd rem-bonus-contract
$ bash build.sh
```

Or, if you already worked with the project:

```bash
$ cd build/rem.bonus && make
```

## Deployment

```bash
$ remcli set contract rembonus 'absolute_path'/remme-guardian-reward-contract/build/rem.bonus --abi rem.bonus.abi -p rembonus
```
