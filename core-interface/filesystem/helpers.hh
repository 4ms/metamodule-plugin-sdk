#pragma once
#include <string>
#include <string_view>

namespace MetaModule::Filesystem
{

// translate_path_to_local(std::string_view path, std::string_view local_path, unsigned num_subdirs = 0);
//
// Translate a path to a file on a computer to a path that works on the MetaModule.
// Works by appending the file name from the foreign path to the provided local path.
// Optionally, one or more parent directories from the foreign path can be appended.
// Typically the local path would be the path to the currently playing patch file.
//
// Useful for when a module stores a path to a sample or wavetable file. The module
// can call this function to translate the stored path into one that will work on the MetaModule.
//
// The most simple way would be to have the user copy all files that a patch uses into the same
// directory as the patch .yml file. Then the module calls this:
// 		std::string mm_path = translate_path_to_local(vcv_computer_path, Patch::get_path(), 0);
// mm_path will point to a file with the same name as the file in `vcv_computer_path`,
// but in the same dir as the currently playing patch file.
//
// The function checks if the path appears to be local already by seeing if it begins with a
// MetaModule volume name. If so, it returns the path unaltered.
//
// Parameters:
// path: the path from a computer that you want to translate
// local_path: the MetaModule path you want to pre-prend
// num_subdirs: sets how many subdirectories to capture from `path` (default 0, max 2)
//
// Examples with different num_subdirs (assume local_path is "sdc:/live-set/"):
// With num_subdirs = 0: /Users/4ms/music/rack/samples/loop.wav => sdc:/live-set/loop.wav
// With num_subdirs = 1: /Users/4ms/music/rack/samples/loop.wav => sdc:/live-set/samples/loop.wav
// With num_subdirs = 2: /Users/4ms/music/rack/samples/loop.wav => sdc:/live-set/rack/samples/loop.wav
//
// More Examples:
// translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/", 0)  			==> sdc:/a.wav
// translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/", 1)  			==> sdc:/samples/a.wav
// translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/patches/", 0)  	==> sdc:/patches/a.wav
// translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/patches/", 1)  	==> sdc:/patches/samples/a.wav
// translate_path_to_local("C:\Users\user\rack\samples\a.wav", "sdc:/patches/", 1)  ==> sdc:/patches/samples/a.wav
// translate_path_to_local("usb:/samples/a.wav", "sdc:/patches/", 1)  				==> usb:/samples/a.wav (path is already local)
// translate_path_to_local("/root/a.wav", "sdc:/patches/", 1)  						==> sdc:/patches/root/a.wav
// translate_path_to_local("/root/a.wav", "sdc:/patches/", 2)  						==> sdc:/patches/root/a.wav

std::string translate_path_to_local(std::string_view path, std::string_view local_path, unsigned num_subdirs = 0);

// Given a path, returns true if it appears to be a valid MetaModule path.
// This only checks if the path starts with a valid volume name and colon,
// it does not check if the file or directories actually exist.
bool is_local_path(std::string_view path);

} // namespace MetaModule::Filesystem
