# System API

This document describes various functions available to query the state of the system or
make simple requests to the system.

## Audio Settings

See [audio/settings.hh](../core-interface/audio/settings.hh)

Namespace: `MetaModule::Audio`

### get_block_size()

```c++
uint32_t get_block_size();
```

Returns the audio block size. Possible values are 16, 32, 64, 128, 256, or 512.

Example usage:

```c++
#include "audio/settings.hh"

auto size = MetaModule::Audio::get_block_size();
```

-- 

## File Browser

File Browsers allow the user to choose a file or directory. The MetaModule
GUI firmware manages the file browser when the user choose a path it forwards that
to your code. 
The File Browser is "asynchronuous", meaning that your code is not blocked while
the user is navigating the browser. Once the user has chosen a file or directory,
a function that you provided will be called with the path as its argument.

Generally these should not be called from the audio context.
But, in some situations it might be OK to do so as long as the user would not
be surprised if the audio stalls or there's a glitch. For example, if the user
clicks a button to load a new sample file, hearing a click after they pick
the new file should not come as a surprise. That said, it's certainly a better
user experience to not have any clicks or glitches, so if possible we recommend
only calling these functions from an AsyncThread, the module's constructor, or
a VCV Rack context menu lambda.

There are three interfaces for using File Browsers. Each one opens up an identical 
File Browser. The Rack interface is designed to make porting from a VCV Rack
module as seamless as possible. The Cardinal interface is identical to the
Cardinal file browser interface, and the Native interface is more natural for
use when creating a new module (not porting an existing one).

In all cases, you must provide a function (typically a lambda) that takes a `char* path` 
argument and acts upon it. To mimic the VCV Rack API, the lambda is required to
free `char*` if it's non-null. Failure to call `free(path)` will result in a memory leak.


There are three basic versions of the File Browser:

### Open File

This returns a path to an existing file.

Native API:

```c++
void async_open_file(std::string_view initial_path,
					 std::string_view filter_extension_list,
					 std::string_view title,
					 std::function<void(char *path)> &&action);
```

Usage:

```c++
#include "filesystem/async_filebrowser.hh"

struct MyModule : CoreProcessor {
    //...
    std::string last_path = "";
    AsyncThread async{this, [this](){ async_thread_runner(); }};

    void async_thread_runner() {
        if (user_clicked_button()) {
            async_open_file(last_path, "wav,WAV,raw,RAW", "Choose a Sample File", 
                [this](char *path) {
                    if (path) {
                        load_file(path);
                        last_path = path;
                        free(path);
                        Patch::mark_patch_modified();
                    }
                }
            );
        }
    }

```
