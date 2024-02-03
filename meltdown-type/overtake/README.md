# Overtake

This repository contains the source code for the paper:
- "Overtake: Achieving Meltdown-type Attacks with One Instruction" (AsianHOST 2023)

You can find the paper at the [Overtake: Achieving Meltdown-type Attacks with One Instruction | IEEE Conference Publication | IEEE Xplore](https://ieeexplore.ieee.org/abstract/document/10409342).

## Tested Environment

- Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
- Intel(R) Core(TM) i7-6800 CPU @ 3.40GHz
- Intel(R) Core(TM) i7-7700 CPU @ 3.60GHz
- Intel(R) Xeon(R) Gold 6133 CPU @ 2.50GHz

## Meta Information

PoC naming rule: `overtake_[attack_short_name]_[instruction]_[covert-channel].c`

- `attack_short_name`: 
    - `md` for `meltdown`
    - `zbl` for `zombieload`
- `instruction`:
    - `cmpsb` for `CMPSB`
    - `cmpxchg` for `CMPXCHG`
- `covert-channel`:
    - `cache_cc` for `cache-based covert channel`

Step for running overtake zbl attack with CMPXCHG instruction and cache-based covert channel:
```shell
# launch the secret process in the background
cd poc/victim_userspace && taskset -c 1 ./secret

# launch the overtaker process in the sibling core
cd poc && make && sudo taskset -c 5 ./zbl.out
# for `zbl` attack, the attacker needs to know a physical address of memory block within itself
```
