#pragma once
#include "util/static_string.hh"
#include <string>

namespace MetaModule::Patch
{

// mark_patch_modified()
// Indicates the currently playing patch has been modified. A red
// dot will appear above the File icon and the user can save the patch file.
// This is only necessary if the internal state changes in a way users would
// expect to be able to recall later (e.g. the user loaded a sample file)
//
// You do not need to call this if a normal parameter or AltParam changed
// (e.g. a button was pressed).
//
// Note that if a user is updating the patch via Wifi from a computer, calling
// this function will prevent further updates until the user Reverts the file.
// So use this only if you're sure the user will want the patch to be marked
// as modified.
void mark_patch_modified();

// get_volume()
// Returns the volume that the currently playing patch is on, as a null-terminated string.
// Will be one of the following:
//   "usb:/" == USB drive
//   "sdc:/" == SD card
//   "nor:/" == Internal storage
//   "ram:/" == Patch is not saved, exists only in RAM
// Future variations of the MetaModule might support more volumes, so keep that in mind.
//
// You can test the path like this:
//    if (Patch::patch_volume().is_equal("usb:/"))
//
// You can use the volume to create a path:
//    std::string tmp_path = std::string(Patch::patch_volume()) + "tmp.json";
//
StaticString<7> get_volume();

// get_path()
// Returns the full path to the patch file as dynamically allocated string.
// Example: "usb:/live-set-01/random-drums.yml"
// Do not use in audio context (since it will allocate)
std::string get_path();

// get_dir()
// Like get_path() but strips the patch name:
// Example: "usb:/live-set-01/"
// Do not use in audio context (since it will allocate)
std::string get_dir();

} // namespace MetaModule::Patch
