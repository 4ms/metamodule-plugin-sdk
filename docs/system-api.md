# System API

This document describes various functions available to query the state of the system or
make simple requests to the system.

## Audio Settings

See [audio/settings.hh](../core-interface/audio/settings.hh)

Namespace: `MetaModule::Audio`

### Audio::get_block_size()

```c++
uint32_t get_block_size();
```

Returns the audio block size. Possible values are 16, 32, 64, 128, 256, or 512.

Example usage:

```c++
#include "audio/settings.hh"
using namespace MetaModule;

auto size = Audio::get_block_size();

```

## Notifications

See [gui/notification.hh](../core-interface/gui/notification.hh)

Namespace: `MetaModule::Gui`

### Gui::notify_user()

```c++
void notify_user(std::string_view message, int duration_ms);
```

Triggers a notification to be displayed to the user at the top of the screen.
The Notification will automatically hide after the given number of
milliseconds. The message should be 255 characters or less (it will be
truncated if longer). Typical durations are 1000-4000 (which is 1 - 4 seconds).

Safe to be called from audio context (but as always, do not make the mistake of
creating or appending to a `std::string` while in the audio context).

Example usage:

```c++
#include "gui/notification.hh"
using namespace MetaModule;

if (file_format_unknown) {
    Gui::notify_user("The file format is not recognized, please choose a different file", 3000);
}

```


## Patch Files

See [patch/patch_file.hh](../core-interface/patch/patch_file.hh)

Namespace: `MetaModule::Patch`

### Patch::mark_patch_modified()

```c++
void mark_patch_modified();
```

Tells the MetaModule system that the current patch file has unsaved changes and
needs to be saved. Calling this will put a red dot over the File menu icon at
the top of the patch. It also will prevent the patch file from being reloaded
automatically if the user updates the patch via Wi-Fi or on the SD Card or USB
drive. 

The main use case for this option is when your module changes something that
needs to be saved in the module's state (`save_state()` for native modules, or
`dataToJson()` for VCV ports).

A common example is the file path for a sample or wavetable file that the user
selects with the File Browser. When a user picks a file to be used with your
module, you should call `mark_patch_modified()` to indicate to the user that
they need to save the patch file. Otherwise the user might not realized they 
have to save the patch in order to have that file loaded the next time they
open the patch.

Be careful not to over-use this. It will prevent the user from syncing their
patch to VCV Rack with the Wi-Fi expander, which disrupts a common workflow.
Only call this function if the user explicitly made a change that must be saved
into the patch file.

Example usage:

```c++
#include "patch/patch_file.hh"
using namespace MetaModule;

if (user_loaded_a_file) {
    Patch::mark_patch_modified();
}

```


### Patch::get_volume()

```c++
StaticString<7> get_volume();
```

Returns the volume that the currently playing patch is on, as a null-terminated string.

Will be one of the following:
   - "usb:/": USB drive
   - "sdc:/": SD card
   - "nor:/": Internal storage
   - "ram:/": Patch is not saved, exists only in RAM

Future variations of the MetaModule might support more volumes, so keep that in mind.

Example usage:

```c++
#include "patch/patch_file.hh"
using namespace MetaModule;

// You can test the path like this (safe to do in any context):
if (Patch::patch_volume().is_equal("ram:/")) {
    //...
}

// You can use the volume to create a path (only safe to use std::string in the
// constructor or an AsyncThread) 

std::string tmp_path = std::string(Patch::patch_volume()) + "tmp.json";

```


*Note:*
`StaticString` is a simple string class that stores an array of characters
without dynamic memory. In this case, `StaticString<7>` is an array of 8 chars
(maxmimum of 7 chars in the string plus 1 null-termination). It easily converts
to/from std::string_view. You can see how `StaticString` works by looking at
the class in [cpputil/util/static_string.hh]

### Patch::get_path()

```c++
std::string get_path();
```

Returns the full path to the patch file as dynamically allocated string.
Example: "usb:/live-set-01/random-drums.yml"
Do not use in audio context (since it will allocate).

Example usage:

```c++
#include "patch/patch_file.hh"
using namespace MetaModule;

#include <filesystem>

// Open a .dat file in the same directory as the patch file (with the same stem, e.g. patch.yml => patch.dat)
std::string patch_path = Patch::get_path();
std::string data_path = std::filesystem::path(patch_path).stem().string() + ".dat";
auto fil = fopen(data_path.c_str(), "w+");


```

## Memory

See [system/memory.hh](../core-interface/system/memory.hh)

Namespace: `MetaModule::System`

### System::free_memory()

```c++
uint32_t free_memory();
```

Returns the free memory available for allocation (malloc, new, etc.)
This memory is not necessarily contigious, so there is no guarentee that you
can allocate a block the size of the value returned. However, if the value is
smaller than what you want to allocate, then you know for sure that your
allocation would fail. Another use for this is to make an allocation that's
a percentage of the available memory (e.g. allocate 25% of the free memory).

Example usage:

```c++
#include "system/memory.hh"
using namespace MetaModule;

std::vector<float> buffer;

auto free_mem = System::free_memory();
buffer.resize(free_mem / 4); // limit our usage to 25% of available memory

```


### System::total_memory()

```c++
uint32_t total_memory();
```

Returns the total memory available to all modules.
This value is subject to change from firmware to firmware version.
On the latest firmware at time of writing (v2.0.12), this returns 304087040
(290MB). Typically the only usage for this would be to compare this to the
result from `free_memory()` to get a sense if memory is nearly full or not.

Example usage:

```c++
#include "system/memory.hh"
using namespace MetaModule;

auto total_mem = System::total_memory();

```

## Random

See [system/random.hh](../core-interface/system/random.hh)

Namespace: `MetaModule::System`

### System::hardware_random()

```c++
uint32_t hardware_random();
```

`hardware_random` returns a true random number generated by hardware with an
analog entropy source. On average, it's about 2-3x slower than calling random().

The hardware takes time to generate new values, so you must call `hardware_random_ready()`
first and make sure it returns true or else you will not get a unique random value.

Example usage:

```c++
#include "system/memory.hh"
using namespace MetaModule;


if (System::hardware_random_ready())
    val = System::hardware_random();

```

### System::hardware_random_ready()

```c++
bool hardware_random_ready();
```

Returns true when the random number hardware is ready to return a new random value. 
If this returns false, then calling `hardware_random()` will return the
previous value.

Example usage: See `hardware_random()`

### System::random()

```c++
uint32_t random();
```

`random` returns a pseudo-random number generated by an algorithm. This is a simple
wrapper for `std::rand()`.

Example usage:

```c++
#include "system/memory.hh"
using namespace MetaModule;


val = System::random();

```


## Time

See [system/time.hh](../core-interface/system/time.hh)

Namespace: `MetaModule::System`

### System::get_ticks()

```c++
uint32_t get_ticks();
```

Returns the number of milliseconds since power-on.

Example usage:

```c++
#include "system/time.hh"
using namespace MetaModule;

if (System::get_ticks() - last_time > 1000) {
    last_time = System::get_ticks();
    do_something_every_second();
}
```



### System::delay_ms()

```c++
void delay_ms(uint32_t ms);
```

Pauses execution for the specified number of milliseconds.
This is almost never a good idea to use in the audio context as 
it will likely cause an audio glitch or underrun.

Example usage:

```c++
#include "system/time.hh"
using namespace MetaModule;

System::delay_ms(1);
```


