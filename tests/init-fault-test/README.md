# Init Fault Test

Test plugin for the firmware's abort rescue (`firmware/system/abort_rescue.cc`).

`init()` registers a module and then faults. The firmware armed a rescue scope before it called `init()`, so it
should roll back to it, remove the module again, and report that the plugin crashed. It must not freeze.

## Build

```
cmake -B build -G Ninja
cmake --build build
```

The plugin is at `build/plugin/InitFaultTest.mmplugin`.

The kind of fault is chosen at configure time with `-DFAULT_TYPE=...`:

| `FAULT_TYPE` | What `init()` does | Firmware handler |
|---|---|---|
| `data` (default) | Writes to an address that is not mapped | Data abort |
| `prefetch` | Calls a function at an address that is not mapped | Prefetch abort |
| `undef` | Executes an undefined instruction (`udf #0`) | Undefined instruction |
| `abort` | Calls `abort()` | The plugin abort hook (`mm_plugin_abort`) |

## With a debugger attached

A debugger that halts the core on exception vectors (vector catch) stops it at the abort vector, before the
firmware's handler runs. The console stops after `faulting now`, with no `[abort]` line, which looks like a
freeze. TRACE32 does this by default. Either resume the core (the handler then runs), or turn it off before
loading the plugin:

```
TrOnchip.Set DABORT OFF
TrOnchip.Set PABORT OFF
TrOnchip.Set UNDEF OFF
```

## Test 1: a fault in init() is rescued

- Copy `InitFaultTest.mmplugin` to the SD card or USB drive, and load it from the Plugins tab.
- Pass:
  - The console shows (for `data`; the others differ in the fault's name and details):

    ```
    [init-fault] init(): registering module InitFault
    [init-fault] init(): faulting now (data)
    [abort] data abort: pc 0x........ addr 0x00000010 fsr 0x........
    [rescue] data abort in proc 3: rolling back
    Unregistering brand 'InitFaultTest' from crashed plugin
    ```

  - The Plugins tab reports "Plugin crashed while initializing", and the plugin is not in the loaded list.
  - There is no "Init Fault Test" brand in the module list.
  - The GUI and audio keep running, and the plugin can be loaded again (with the same result).
- Fail: the GUI freezes, or the console shows `FAIL: init() is still running after the fault`.

Repeat with each `FAULT_TYPE`.

## Test 2: a fault is rescued while the other core holds an allocator lock

The rescue is refused if the process that faulted holds the malloc lock or the plugin arena lock: rolling back
would leave it locked forever. A lock held by a different process must not stop the rescue.

Plugins are loaded by the GUI thread on core 1 (process 3). The only processes that can hold a lock at the
moment it faults are on core 0, so this test needs something on core 0 that allocates all the time. The Lock
Test's AsyncThreads do that.

- Set up `../lock-deadlock-test` as its README describes: load `LockDeadlockTest.mmplugin`, and play
  `LockTest_x4.yml` with Auto Rebalance off. Modules 1 and 2 must show `async 0`.
- Turn knob C (AsyncThread allocations) fully up, and A, B and D down.
- Load `InitFaultTest.mmplugin` 20 times or more.
- Firmware that checks who holds the lock: every load is rescued, as in test 1. The Lock Tests keep running
  with `Errors 0/0`.
- Firmware that only checks whether the lock is held: some of the loads freeze the GUI, with

  ```
  [rescue] data abort in proc 3: allocator lock held, cannot rescue
  ```

  on the console.

Then raise D until the CPU load is near the limit and repeat: audio interrupts the AsyncThreads on core 0 while
they hold the lock, so the lock is held for longer.
