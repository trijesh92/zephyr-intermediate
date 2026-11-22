# L6-TASK1: Latency investigation and tracing

## before correction (priorities: control 7, maintenance 4)

### Shell outputs
```
*** Booting Zephyr OS build v4.4.2 ***
[00:00:00.000,000] <inf> homework: === L6 Homework: Runtime Investigation ===
[00:00:00.000,000] <inf> homework: Control work must start within 10 ms
[00:00:00.000,000] <inf> homework: Inspect, measure, trace, explain, and correct the delay
[00:00:00.545,000] <inf> homework: [CONTROL] processed seq=0
[00:00:00.795,000] <inf> homework: [CONTROL] processed seq=1
[00:00:01.045,000] <inf> homework: [CONTROL] processed seq=2
[00:00:01.045,000] <wrn> homework: Skipped 2 messages
[00:00:01.045,000] <wrn> homework: [CONTROL] deadline_miss count=3 seq=2 latency=45ms
[00:00:01.295,000] <inf> homework: [CONTROL] processed seq=3
[00:00:01.545,000] <inf> homework: [CONTROL] processed seq=4
```

```
uart:~$ kernel thread list
Scheduler: 104 since last call
Threads:
 0x200201d0 maintenance
        options: 0x0, priority: 4 timeout: 0
        state: pending, entry: 0x8000679
        stack size 2048, unused 1800, usage 248 / 2048 (12 %)

 0x20020288 control
        options: 0x0, priority: 7 timeout: 0
        state: pending, entry: 0x8000699
        stack size 2048, unused 1696, usage 352 / 2048 (17 %)

*0x20020850 shell_uart
        options: 0x0, priority: 14 timeout: -9223372036854775808
        state: queued, entry: 0x8005a8d
        stack size 2048, unused 952, usage 1096 / 2048 (53 %)

 0x20020350 logging
        options: 0x0, priority: 14 timeout: -9223372036854775808
        state: pending, entry: 0x80019a1
        stack size 768, unused 376, usage 392 / 768 (51 %)

 0x20020be0 idle
        options: 0x1, priority: 15 timeout: 0
        state: , entry: 0x800d457
        stack size 320, unused 200, usage 120 / 320 (37 %)
```

```
uart:~$ kernel thread stacks
0x200201d0 maintenance                      (real size 2048):   unused 1800    usage  248 / 2048 (12 %)
0x20020288 control                          (real size 2048):   unused 1696    usage  352 / 2048 (17 %)
0x20020850 shell_uart                       (real size 2048):   unused  912    usage 1136 / 2048 (55 %)
0x20020350 logging                          (real size  768):   unused  376    usage  392 /  768 (51 %)
0x20020be0 idle                             (real size  320):   unused  200    usage  120 /  320 (37 %)
0x20023fc0 IRQ 00                           (real size 2048):   unused 1600    usage  448 / 2048 (21 %)

```