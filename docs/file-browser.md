## File Browsers

See [filesystem/async_filebrowser.hh](../core-interface/filesystem/async_filebrowser.hh)

The Open File Browser allows the user to choose a file or directory. The MetaModule
GUI firmware displays manages the file browser, allowing the user to view the 
contents of the various attached drives, and ultimately selecting a file or a directory.
The path to this file or directory will be provided your module's code.

The Save File Browser is similar to the Open File Browser, but it lets the user select a directory
and also enter/edit a file name. The file extention is typically hard-set.

Both File Browsers are "asynchronuous", meaning that your code is not blocked while
the user is navigating the browser. Once the user has chosen a file or directory,
a function or lambda that you provided will be called with the path as its argument.
This function or lambda is called the callback.

It's safe to call the File Browser functions from the audio context because all
that happens is the GUI is given your callback and notified that it needs to
open the browser. Your callback will be called in the GUI context, so it can
load a file, allocate memory, or do anything else that's not permitted in the
audio context.

There are three interfaces for using File Browsers. Each one opens up an identical 
File Browser. The Rack interface is designed to make porting from a VCV Rack
module as seamless as possible. The Cardinal interface is identical to the
Cardinal file browser interface, and the Native interface is more natural when
creating a new module (not porting an existing one).

In all cases, you must provide a callback function (typically a lambda) that
takes a `char* path` argument. This callback will be called with the path if/when
the user chooses a path. If the user presses the Back button to exit the Browser
without choosing a file, your callback will not be called.

To mimic the VCV Rack API, the lambda is required to free the `char*` path.
Failure to call `free(path)` will result in a memory leak.

There are three basic versions of the File Browser:

### Native API:

#### Open File Browser

This will tell the GUI to open a File Browser and let the user pick an existing file. 
The callback will be provided the path to this file.


```c++
void async_open_file(std::string_view initial_path,
					 std::string_view filter_extension_list,
					 std::string_view title,
					 std::function<void(char *path)> &&action);
```

`initial_path` is the directory that the File Browser should start in.
If it's empty, the Brower will start in the last directory viewed (which may or may not 
be the last directory YOUR module viewed).

`filter_extension_list` is a comma-separated list of allowed extensions. 
  - Example: `"wav, WAV, raw, RAW"`: only files ending in .wav, .WAV, .raw, or
    .RAW will be shown.
There are some special strings to be aware of:
  - `"*.*"`: If this is contained anywhere in `filter_extension_list`, then the
    Browser will show all files.
  - `"*/"`: If `filter_extension_list` is exactly this, then the user will be
    able to select a directory instead of a file. The subtitle will be set to
    "Folders only:"

`title`: This string will be shown at the top.

`action`: this is a function or lambda that does something with the path the
user chose. If the user clicked cancel or a disk error happened, this may be
set to nullptr. In that case your lambda or function should take no action.
Otherwise, you can assume the path is valid. You must call `free(path)` when
done.


#### Open Directory Browser

This will tell the GUI to open a File Browser and let the user pick an existing directory.
The callback will be provided the path to this directory.

```c++
void async_open_dir(std::string_view initial_path, 
                    std::string_view title, 
                    std::function<void(char *path)> &&action);
```

The parameters `initial_path`, `title`, and `action` have the same meaning as they do in `async_open_file` (see above).


#### Save File Browser

This will tell the GUI to open a Save File Browser and let the user pick an
existing directory into which a file should be saved. The user can also enter a
file name. The file extension cannot be editted.

The callback will be provided the path to this file (which does not yet exist).

```c++
void async_save_file(std::string_view initial_path,
					 std::string_view filename,
					 std::string_view extension,
					 std::function<void(char *path)> &&action);
```

`initial_path` is the directory that the Save File dialog should start in.
If it's empty, the Brower will start in the last directory any Save File Browser was in (which may or may not 
be the last directory YOUR module was using).

Example Usage:

```c++
#include "filesystem/async_filebrowser.hh"

struct MyModule : CoreProcessor {
    //...
    std::string last_path = "";
    AsyncThread async{this, [this](){ async_thread_runner(); }};

    void async_thread_runner() {
        if (user_clicked_load_file_button) {

            async_open_file(last_path, "wav,WAV,raw,RAW", "Choose a Sample File", 
                [this](char *path) {
                    if (path) {
                        last_path = path;
                        load_file(path);
                        free(path);
                        Patch::mark_patch_modified();
                    }
                }
            );

        }

        if (user_clicked_load_folder_button) {
            async_open_dir(last_path, "Choose a Folder:", 
                [this](char *path) {
                    if (path) {
                        last_path = path;
                        set_directory(path);
                        free(path);
                        Patch::mark_patch_modified();
                    }
                }
            );
        }

        if (user_clicked_save_button) {
            async_save_file(last_path, "Untitled", ".wav",
                [this](char *path) {
                    if (path) {
                        last_path = path;
                        save_file(path);
                        free(path);
                    }
                }
            );
        }

    }

```


#### Rack API:

```c++
void async_osdialog_file(osdialog_file_action action,
						 const char *path,
						 const char *filename,
						 osdialog_filters *filters,
						 std::function<void(char *path)> &&action_function);
```


