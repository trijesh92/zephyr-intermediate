# L6-TASK1: Latency investigation and tracing

## After correction (priorities: control 4, maintenance 7)

### Shell outputs
```
*** Booting Zephyr OS build v4.4.2 ***
[00:00:00.000,000] <inf> homework: === L6 Homework: Runtime Investigation ===
[00:00:00.000,000] <inf> homework: Control work must start within 10 ms
[00:00:00.000,000] <inf> homework: Inspect, measure, trace, explain, and correct the delay
[00:00:00.500,000] <inf> homework: [CONTROL] processed seq=0
[00:00:00.500,000] <inf> homework: [CONTROL] event_done seq=0 latency=0ms
[00:00:00.750,000] <inf> homework: [CONTROL] processed seq=1
[00:00:00.750,000] <inf> homework: [CONTROL] event_done seq=1 latency=0ms
[00:00:01.000,000] <inf> homework: [CONTROL] processed seq=2
[00:00:01.000,000] <inf> homework: [CONTROL] event_done seq=2 latency=0ms
[00:00:01.250,000] <inf> homework: [CONTROL] processed seq=3
[00:00:01.250,000] <inf> homework: [CONTROL] event_done seq=3 latency=0ms
[00:00:01.500,000] <inf> homework: [CONTROL] processed seq=4
[00:00:01.500,000] <inf> homework: [CONTROL] event_done seq=4 latency=0ms
```

```
uart:~$ kernel thread list
Scheduler: 2 since last call
Threads:
 0x200201d0 maintenance
        options: 0x0, priority: 7 timeout: 0
        state: pending, entry: 0x8000669
        stack size 2048, unused 1800, usage 248 / 2048 (12 %)

 0x20020288 control
        options: 0x0, priority: 4 timeout: 0
        state: pending, entry: 0x8000689
        stack size 2048, unused 1760, usage 288 / 2048 (14 %)

*0x20020850 shell_uart
        options: 0x0, priority: 14 timeout: -9223372036854775808
        state: queued, entry: 0x8005a6d
        stack size 2048, unused 952, usage 1096 / 2048 (53 %)

 0x20020350 logging
        options: 0x0, priority: 14 timeout: -9223372036854775808
        state: pending, entry: 0x8001981
        stack size 768, unused 376, usage 392 / 768 (51 %)

 0x20020be0 idle
        options: 0x1, priority: 15 timeout: 0
        state: , entry: 0x800d437
        stack size 320, unused 200, usage 120 / 320 (37 %)
```

```
uart:~$ kernel thread stacks
0x200201d0 maintenance                      (real size 2048):   unused 1800    usage  248 / 2048 (12 %)
0x20020288 control                          (real size 2048):   unused 1760    usage  288 / 2048 (14 %)
0x20020850 shell_uart                       (real size 2048):   unused  912    usage 1136 / 2048 (55 %)
0x20020350 logging                          (real size  768):   unused  376    usage  392 /  768 (51 %)
0x20020be0 idle                             (real size  320):   unused  200    usage  120 /  320 (37 %)
0x20023fc0 IRQ 00                           (real size 2048):   unused 1592    usage  456 / 2048 (22 %)
```
