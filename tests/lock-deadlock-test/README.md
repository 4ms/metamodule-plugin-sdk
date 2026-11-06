# Lock Deadlock Test

Test plugin for deadlocks in the firmware's newlib locks (`firmware/system/lock.cc`), see
[#635](https://github.com/4ms/metamodule/issues/635).

The **Lock Test** module allocates from the plugin arena in three contexts at once. Each has a knob for how many
allocations it does per call:

| Knob | Context | Runs in |
|---|---|---|
| Left | Text display (`get_display_text()`) | The ReadPatchGuiElements interrupt on core 1, while the display is on screen |
| Middle | Graphic display (`draw()`) | The GUI thread (Core1Main) |
| Right | AsyncThread | The core the module is assigned to |

The text display shows how many times each one has run, and which core the AsyncThread is on (it's also printed to
the console). The graphic display draws a moving bar, and `process()` blinks the light.

**A freeze** is when everything stops at once: the counters, the bar, the light, the encoder and the USB console. There
is no crash report. Audio stops too.

## Build

```
cmake -B build -G Ninja
cmake --build build
```

The plugin is at `build/plugin/LockDeadlockTest.mmplugin`.

## Tests

Play the patch and open the module's view, so its text display is on screen. The text display is only read while
it's visible.

### 1. The display text interrupts the GUI while the GUI is allocating

The text display's allocations run in an interrupt that can't be held off while the GUI holds the arena lock (SGIs
cannot be disabled on the GIC-400).

- Patch: one Lock Test.
- Knobs: left and middle up (for example 100 and 1000), right at 0.
- Before the fix: freezes, probably within seconds to minutes.
- Fixed: runs indefinitely.

### 2. Core 0 deadlock

The AsyncThread on core 0 holds the arena lock and audio interrupts it. Audio then waits for the display text read on
core 1, which is waiting for the lock.

- Patch: Lock Test, plus heavy modules to raise the audio load. Check the text display's "Async core": it must say
  0. If it says 1, add or remove modules until it's on core 0.
- Knobs: left and right up, middle at 0. Raise the left knob until the CPU load is near the limit: the display text
  read has to still be running when the next audio block starts.
- Before the fix: freezes.
- Fixed with the try-lock change (`newlib-lock-fix`): runs indefinitely.
- With only the SPI change (`newlib-lock-fix-spi`): expected to still freeze.

### 3. Firmware heap instead of the plugin arena (as reported in #635)

This uses the newlib malloc lock instead of the arena lock, and needs no plugin.

- Patch: RackCore's MIDIToCVInterface. Send it a MIDI message so its display text isn't empty, and keep the patch
  on screen.
- Send console commands over USB continuously (the GUI allocates while it handles them).
- Before the fix: froze after 7.5 minutes in #635.

### Audio cost

Compare the peak CPU load with the knobs up against all knobs at 0, on each firmware. A fix should not raise it.
