# Lock Deadlock Test

Test plugin for deadlocks and arena corruption in the firmware's newlib locks (`firmware/system/lock.cc`), see
[#635](https://github.com/4ms/metamodule/issues/635) and [#637](https://github.com/4ms/metamodule/issues/637).

The **Lock Test** module allocates from the plugin arena in three contexts at once. Each has a knob for how many
allocations it does per call. A fourth knob adds DSP work to `process()`:

| Knob | Patch knob | Context | Runs in |
|---|---|---|---|
| Top left | A | Text display (`get_display_text()`) | The ReadPatchGuiElements interrupt on core 1, while the display is on screen |
| Top middle | B | Graphic display (`draw()`) | The GUI thread (Core1Main), while the module is on screen |
| Top right | C | AsyncThread | The core the module is assigned to |
| Bottom left | D | DSP work in `process()` | The audio interrupt of the core the module is assigned to |

The DSP work reads and writes a 16 kB table, and the amount varies randomly from half to one and a half times the
knob's value, so the audio interrupts end at different times relative to the other contexts. The input is mixed into
the DSP work and the output is its result (quiet), so cables between modules on different cores add the cable
processing that a real patch has.

The text display shows:

- How many times each context has run.
- `Core: audio N async N`: which core the module's audio and AsyncThread run on. The AsyncThread's core is also
  printed to the console.
- `Errors N/M`: corrupted strings found by this module, and by all Lock Tests. Each context checks that its strings
  still hold what it wrote before it replaces them, so a block handed to two contexts at once (#637) shows up here,
  usually before it crashes. The graphic display and AsyncThread also print each error to the console. The text
  display only counts them, because printing would take the stdio lock there.

The graphic display draws a moving bar, and `process()` blinks the light.

**A freeze** is when everything stops at once: the counters, the bar, the light, the encoder and the USB console. There
is no crash report. Audio stops too.

**Corruption** is a rising error count, a data abort in the plugin arena's TLSF allocator (`mm_tlsf_*`,
`remove_free_block`, `insert_free_block`), often with an address that is ASCII text, or both.

## Build

```
cmake -B build -G Ninja
cmake --build build
```

The plugin is at `build/plugin/LockDeadlockTest.mmplugin`.

## The test patch

`patches/LockTest_x4.yml` has four Lock Tests: 1 and 2 on core 0, 3 and 4 on core 1. The core assignment is stored in
the patch (`module_cores`), so the load balancer doesn't move them. The cables go 1 → 3 → 2 → 4 → 1, so each one
crosses between the cores. Patch knobs A to D are mapped to the same knob on all four modules.

Setup:

- Copy `LockDeadlockTest.mmplugin` and `LockTest_x4.yml` to the SD card or USB drive, and load the plugin.
- Set Auto Rebalance (Settings > Audio) to **Off**. Otherwise an overload, which is likely while you raise knob D,
  makes the firmware re-balance and save a new assignment.
- Turn knobs A to D fully down, then load the patch.
- Stay on the patch view: it reads all four text displays and draws all four graphic displays. A module's own view
  only runs that one module's displays.
- Check each module's `Core:` line. Modules 1 and 2 must say `audio 0 async 0`, and 3 and 4 `audio 1 async 1`. If
  not, the stored assignment wasn't used: check Auto Rebalance, and reload the patch.

Which contexts end up competing for a lock:

| Context | Core |
|---|---|
| Text displays (A), all modules | 1, interrupt |
| Graphic displays (B), all modules | 1, GUI thread |
| AsyncThreads (C) of modules 1 and 2 | 0 |
| AsyncThreads (C) of modules 3 and 4 | 1 |
| Audio (D) of modules 1 and 2 | 0 |
| Audio (D) of modules 3 and 4 | 1 |

## Tests

Run each one for as long as you can, at least 10 minutes, and longer for the soak test. Note the firmware build in
each result.

The firmware builds, from oldest to newest:

| Build | What's fixed |
|---|---|
| `main` (before the fixes) | nothing |
| `newlib-lock-fix` | the lock races, and the display text no longer waits for a lock (#635, and the core 0 deadlock) |
| `mmu-shareable` | as above, plus the locks work across cores (#637) |

### 1. The display text interrupts a core 1 lock holder (#635)

The text display's allocations run in an interrupt that can't be held off while the GUI, or an AsyncThread on core 1,
holds the arena lock (SGIs can't be disabled on the GIC-400).

- Knobs: A and B up, C and D down. Then raise C too: that adds the core 1 AsyncThreads as lock holders.
- `main`: freezes, probably within seconds to minutes.
- `newlib-lock-fix` and `mmu-shareable`: runs. The text counters may climb more slowly with B up, because a read
  that finds a lock busy is skipped.

### 2. The arena on two cores at once (#637)

The AsyncThreads of modules 1 and 2 allocate on core 0 while the graphic and text displays allocate on core 1.

- Knobs: B and C up, A and D down. Then raise A too.
- `main` and `newlib-lock-fix`: errors and a data abort, usually within seconds (`main` may freeze first).
- `mmu-shareable`: runs, with `Errors 0/0`.

### 3. Core 0 deadlock

An AsyncThread on core 0 holds the arena lock and audio interrupts it. On `main`, audio then waits for the display
text read on core 1, which is waiting for that lock.

- Knobs: A and C up, B down. Raise D until the CPU load is near the limit: the display text read has to still be
  running when the next audio block starts.
- `main`: freezes.
- `newlib-lock-fix` and `mmu-shareable`: runs.

### 4. Soak

- Knobs: A, B and C up. Every few minutes move D between down and near the limit, and move A, B and C between
  halfway and up, so the timing keeps changing.
- `mmu-shareable`: runs for an hour or more, with `Errors 0/0`, no freeze and no abort.

### 5. Firmware heap instead of the plugin arena (as reported in #635)

This uses the newlib malloc lock instead of the arena lock, and needs no plugin.

- Patch: RackCore's MIDIToCVInterface. Send it a MIDI message so its display text isn't empty, and keep the patch
  on screen.
- Send console commands over USB continuously (the GUI allocates while it handles them).
- `main`: froze after 7.5 minutes in #635.

### Audio cost

With the same knob settings, compare the CPU load on each firmware. A fix should not raise it.
