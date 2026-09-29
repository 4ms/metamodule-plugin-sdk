# Developer Console over USB

The MetaModule has a text console that you can open over USB from your computer. It
shows anything your plugin prints with `printf()`, along with messages from the
MetaModule itself.

The console is especially useful with Developer Mode, which adds a USB drive for
installing plugins (see [Developer Drive](developer-drive.md)) and extra on-screen
error messages about your plugin.

## Connecting

1. On the MetaModule, go to Preferences > USB:
   - Set **USB Mode** to `Auto` or `Device Only`
   - Set **Device Mode** to `MIDI` (not `Video`). The console is part of the same USB
     device as the MIDI port
   - Recommended (but optional), enable **Developer Mode** (see [below](#developer-mode)). You will
     see more error messages if you have this enabled.
2. Plug a USB cable from the MetaModule's USB-C jack to your computer.

The console shows up as a serial port:

| OS | Port |
| --- | --- |
| macOS | `/dev/cu.usbmodem*` |
| Linux | `/dev/ttyACM0` (or another number) |
| Windows | A `COM` port (see Device Manager) |

Open it with any serial terminal program. The baud rate doesn't matter (it's a USB
serial port), so any setting works. For example:

```
screen /dev/cu.usbmodemXXXX
```

```
minicom -c on -D /dev/cu.usbmodemXXXX
```

```
picocom /dev/cu.usbmodemXXXX
```

On Windows, PuTTY or Tera Term work.

Turn on the "Line Feed" or "Carriage Return" option in your terminal program (in
minicom it's `Ctrl-A` or `Opt-Z` then `U`). Otherwise, each new line won't
return to the leftmost column.

If the port disappears and comes back (for example, when you turn Developer Mode on
or off, or after installing a plugin from the developer drive), you may need to
reconnect your terminal program.

## Printing from your plugin

Anything your plugin prints with `printf()` (or writes to `stdout`) appears in the
console. This is usually the quickest way to debug a plugin on hardware:

```c++
printf("Loaded sample '%s': %u frames\n", name.c_str(), num_frames);
```

End each message with `\n`: output is sent a line at a time.

Output is buffered, so `printf()` returns right away and the console sends it in the
background. But the console can only send so fast. If you print faster than that
(e.g. every time `process()` runs), the buffer fills up, and then `printf()` waits for
the console to catch up, which causes audio dropouts. Or some console output
might be dropped, and you'll see:

```
<console overrun>
```

So in `process()`, print only occasionally: when something changes, or every few
thousand samples. And remember to remove the printf() before you release your plugin!

## What else you'll see

The MetaModule prints messages about what it's doing: loading plugins, installing
from the developer drive, and so on.

When you open a module whose `plugin-mm.json` has mistakes in its element `groups`,
`order`, or `names`, each mistake is printed, like this (see
[Element groups](element-groups.md)):

```
Module MyBrand:Mixer: plugin-mm.json: group 'Channel 1' has no element 'Chanel 1 Level'
```

## Building firmware with logging enabled 

Normal firmware releases leave out most of the MetaModule's diagnostic messages
(most errors, warnings and info) to save space and time. Your plugin's
`printf()` output always appears. Firmware built with logging enabled
(`LOG_LEVEL`, see the firmware's `docs/firmware-debugging.md`) prints much
more. To enable this, you need to build the firmware. In short:

```bash
git clone --recurse-submodules https://github.com/4ms/metamodule
cd metamodule/firmware
make configure  # <<< This defaults to LOG_LEVEL=DEBUG
make
ls -l build/metamodule-firmware-*-firmware-assets.zip
# Unzip it, copy the metamodule-firmware dir to an SD card or USB drive, and install it on the MetaModule normally
```

For more details, see [MetaModule firmware building](https://github.com/4ms/metamodule/blob/main/docs/firmware-building.md)
and [MetaModule firmware debugging](https://github.com/4ms/metamodule/blob/main/docs/firmware-debugging.md#console-output-printf-debugging)

## Color

The MetaModule has three processor cores, and all of them print to the console. Lines
from different cores are interleaved, but lines ending in `\n` aren't mixed together.
To see which core printed each line, type `col` and press enter.
Each core's output is then colored: A7-main (audio only): blue, A7-aux (audio and GUI): green, and
M4 (low-level): yellow.

## Troubleshooting

**No serial port appears.** Check that Device Mode is `MIDI`, not `Video`, and that USB
Mode isn't `Host Only`. Try re-plugging the cable.

**The port appears but nothing is printed.** Check that your plugin's messages end in
`\n`. Remember that normal firmware releases print very little on their own: try
printing something from your plugin, or type `help` and press enter.

**Lines are staggered across the screen.** Turn on the "Line Feed" or "Carriage
Return" option in your terminal program.

**`<console overrun>` appears.** Something printed faster than the console can
send, so some output was dropped. Throttle back your prints (use a counter to
print once per second rather than every sample, for example).
