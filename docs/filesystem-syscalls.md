# Filesystem Calls

The MetaModule uses newlib for libc. This provides standard library filesystem functions like `fopen`, `fread`, `fclose`.

You can use these exactly as they are used on a desktop computer. However, there are some things to keep in mind:

- Paths are different. Paths start with a the volume name (3 characters), then a colon, then a slash. 
  The volumes available on the MetaModule are:
   - `sdc:/` -- The SD Card drive.
   - `usb:/` -- The USB drive.
   - `ram:/` -- The internal RAM disk. This is where plugins live, as well as
     unsaved patches. Everything in here is temporary. You could create temp
     files here, but it's usually a better idea to just store temp data in
     memory as normal C/C++ variables. If you need shared global state, a small
     file could work (like a lock file used in Linux systems).
   - `nor:/` -- NOR Flash (internal storage). There is a ~2MB volume on this
     flash chip that can hold patch files and other data. The filesystem is
     LFS, but you don't need to consider that when using the drive. It's highly
     recommended to avoid using this drive except for very tiny files (and not
     a lot of files). Users will want to save their patches here, so do not
     fill this up with module or patch data.


- Users might eject a disk at any time. Always check for errors when doing any
  file system access and handle them appropriately.

- Filesystem access should never be done from the audio context.

- Disk access is slow. The MetaModule was not designed to read/write to disks
  quickly, like a desktop computer would be designed. Also, SD Cards are by
  nature slow to read and write. Users might be using a slow or heavily
  fragmented disk. USB drives vary widely in performance. Multiple modules may
  be requesting disk access at the same time as your module. All these factors
  mean it's not surprising if a read request occasionally takes up to 4 or 5
  seconds even for just a few kB of data. You must plan for stalls and delays.
  If you have large files, buffering data to/from RAM is a good idea.

## Helper functions

See [filesystem/helpers.hh](../core-interface/filesystem/helpers.hh)

Namespace: `MetaModule::Filesystem`

### Filesystem::translate_path_to_local()

```c++
std::string translate_path_to_local(std::string_view path, std::string_view local_path, unsigned num_subdirs = 0);
```

Translate a path to a file on a computer hard drive, to a path that works on the MetaModule.

The common use case is when a module stores a path to a sample or wavetable
file. The module can call this function to translate the stored path into one
that will work on the MetaModule.

The function works by appending the file name from the computer path to the
provided local path. Typically the local path would be the path to the
currently playing patch file.

Stated another way: you give it a path to a file on a hard drive, and it
creates a path to a file with the same name, but in the patch directory.
From the user's perspective, they would need to copy the file into the same directory
as the patch .yml file on their USB drive or SD Card, and then everything will work 
as expected.

As an optional parameter, one or more parent directories from the computer path can be appended.
This lets the module access files in a sub-directory that's in the same directory as the 
patch file (for example `samples/`).

The function checks if the path appears to be local already by seeing if it begins with a
MetaModule volume name. If so, it returns the path unaltered. This allows the module 
to always process all file paths with `translate_path_to_local`.

**Parameters:**
- `path`: the path from a computer that you want to translate. If the path is already local, it will be returned.
- `local_path`: the MetaModule path you want to pre-prend. Typically this is `Patch::get_path()`.
- `num_subdirs`: sets how many subdirectories to capture from `path` (default 0, max 2)

This function allocates strings, and is not safe to call from the audio context.

**Example usage:**

The most simple way would be to have the user copy any files that a patch uses into the same
directory as the patch .yml file. Then the module calls this:

```c++
std::string mm_path = translate_path_to_local(vcv_computer_path, Patch::get_path());
```
mm_path will point to a file with the same name as the file in `vcv_computer_path`,
but in the same dir as the currently playing patch file.

```c++
#include "filesystem/helpers.hh"
using namespace MetaModule;

class MyModule : rack::engine::Module {
    
    //...

	void dataFromJson(json_t* rootJ) override {
		json_t* samplePathJ = json_object_get(rootJ, "sampleFilePath");
		if (samplePathJ) {
            // Get the path to a file on a computer hard drive, e.g. /Users/Me/rack/samples/loop.wav
			std::string samplePath = json_string_value(samplePathJ);

            // Translate the path to "usb:/loop.wav"
            std::string localSamplePath = translate_path_to_local(samplePath, Patch::get_path());

            load_sample_file(localsamplePath);
        }
	}


```

** Examples with different num_subdirs**

`translate_path_to_("/Users/4ms/music/rack/samples/loop.wav", "sdc:/live-set/", num_subdirs)`
- num_subdirs = 0: => sdc:/live-set/loop.wav
- num_subdirs = 1: => sdc:/live-set/samples/loop.wav
- num_subdirs = 2: => sdc:/live-set/rack/samples/loop.wav

** More Examples: **
- `translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/", 0)  			==> sdc:/a.wav`
- `translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/", 1)  			==> sdc:/samples/a.wav`
- `translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/patches/", 0)  	==> sdc:/patches/a.wav`
- `translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/patches/", 1)  	==> sdc:/patches/samples/a.wav`
- `translate_path_to_local("C:\Users\user\rack\samples\a.wav", "sdc:/patches/", 1)  ==> sdc:/patches/samples/a.wav`
- `translate_path_to_local("usb:/samples/a.wav", "sdc:/patches/", 1)  				==> usb:/samples/a.wav (path is already local)`
- `translate_path_to_local("/root/a.wav", "sdc:/patches/", 1)  						==> sdc:/patches/root/a.wav`
- `translate_path_to_local("/root/a.wav", "sdc:/patches/", 2)  						==> sdc:/patches/root/a.wav`

### Filesystem::is_local_path()

```c++
bool is_local_path(std::string_view path);
```

Given a path, returns true if it appears to be a valid MetaModule path.

This only checks if the path starts with a valid volume name and colon,
it does not check if the file or directories actually exist or if the
volume is mounted.

Valid volume names (with colon appended) are:
- `sdc:`
- `usb:`
- `nor:`
- `ram:`

This function is safe to call in the audio context.
