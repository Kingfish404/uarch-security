# MDS Attack Code

MDS means Microarchitectural Data Sampling.

## MDS Attack List

- [x] RIDL
- [ ] Fallout
- [x] Zombieload
- [ ] CacheOut
- [x] Medusa

## Other Attack List

- [x] Meltdown
- [x] Spectre

## Platform

Code runing platform:

```bash
# lscpu
Architecture:        x86_64
CPU op-mode(s):      32-bit, 64-bit
Byte Order:          Little Endian
CPU(s):              8
On-line CPU(s) list: 0-7
Thread(s) per core:  2
Core(s) per socket:  4
Socket(s):           1
NUMA node(s):        1
Vendor ID:           GenuineIntel
CPU family:          6
Model:               94
Model name:          Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
Stepping:            3
CPU MHz:             800.395
CPU max MHz:         4000.0000
CPU min MHz:         800.0000
BogoMIPS:            6799.81
Virtualization:      VT-x
L1d cache:           32K
L1i cache:           32K
L2 cache:            256K
L3 cache:            8192K
NUMA node0 CPU(s):   0-7
Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 movbe popcnt aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb invpcid_single tpr_shadow vnmi flexpriority ept vpid ept_ad fsgsbase tsc_adjust bmi1 hle avx2 bmi2 erms invpcid rtm rdseed adx clflushopt intel_pt xsaveopt xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp

# lscpu -e
CPU NODE SOCKET CORE L1d:L1i:L2:L3 ONLINE MAXMHZ    MINMHZ
0   0    0      0    0:0:0:0       yes    4000.0000 800.0000
1   0    0      1    1:1:1:0       yes    4000.0000 800.0000
2   0    0      2    2:2:2:0       yes    4000.0000 800.0000
3   0    0      3    3:3:3:0       yes    4000.0000 800.0000
4   0    0      0    0:0:0:0       yes    4000.0000 800.0000
5   0    0      1    1:1:1:0       yes    4000.0000 800.0000
6   0    0      2    2:2:2:0       yes    4000.0000 800.0000
7   0    0      3    3:3:3:0       yes    4000.0000 800.0000
```

## Reference

- https://meltdownattack.com/
- https://zombieloadattack.com/
- https://github.com/vusec/ridl
- https://github.com/medusajs/medusa
- https://github.com/IAIK/ZombieLoad
- https://github.com/yadav-sachin/spectre-attack-image
- https://seedsecuritylabs.org/Labs_16.04/System/Meltdown_Attack/
