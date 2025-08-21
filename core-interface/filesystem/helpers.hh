#pragma once
#include <string>
#include <string_view>

namespace MetaModule::Filesystem
{

// Translate a path to a file on a computer to a path that works on the MetaModule.
// Works by extracting the file name (and optionally one or more parent directories)
// from the path, and pre-pending a valid MetaModule path (typically the path to the
// currently playing patch).
//
// Useful for when a module saves a path to a sample or wavetable file, you can use this
// to translate that path to one that's relative to the currently playing patch.
// The most simple use case is to have the user copy all files that a patch uses into the same
// directory as the patch .yml file. Then call it like:
// 		std::string mm_path = translate_path_to_local(vcv_computer_path, Patch::get_path(), 0);
//
// The function checks if the path appears to be local already. If so, it returns the path unaltered.
// This is useful if you don't know if the path was saved by VCV or by the MetaModule.
//
// Parameters:
// path: the path from a computer that you want to translate
// local_path: the MetaModule path you want to pre-prend
// num_subdirs: sets how many subdirectories to capture from `path` (default 0, max 2)
//
// Examples with if local_path is "sdc:/live-set/":
// With num_subdirs = 0:
// 		/Users/4ms/music/rack/samples/loop.wav => sdc:/live-set/loop.wav
// With num_subdirs = 1:
// 		/Users/4ms/music/rack/samples/loop.wav => sdc:/live-set/samples/loop.wav
// With num_subdirs = 2:
// 		/Users/4ms/music/rack/samples/loop.wav => sdc:/live-set/rack/samples/loop.wav
//
// More Examples:
// translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/", 0)  			==> sdc:/a.wav
// translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/", 1)  			==> sdc:/samples/a.wav
// translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/patches/", 0)  	==> sdc:/patches/a.wav
// translate_path_to_local("/Users/user/rack/samples/a.wav", "sdc:/patches/", 1)  	==> sdc:/patches/samples/a.wav
// translate_path_to_local("C:\Users\user\rack\samples\a.wav", "sdc:/patches/", 1)  ==> sdc:/patches/samples/a.wav
// translate_path_to_local("usb:/samples/a.wav", "sdc:/patches/", 1)  				==> usb:/samples/a.wav (path is already local)
// translate_path_to_local("/root/a.wav", "sdc:/patches/", 1)  	==> sdc:/patches/root/a.wav
// translate_path_to_local("/root/a.wav", "sdc:/patches/", 2)  	==> sdc:/patches/root/a.wav

std::string translate_path_to_local(std::string_view path, std::string_view local_path, unsigned num_subdirs = 0);

// Given a path, returns true if it appears to be a valid MetaModule path.
// This only checks if the path starts with a valid volume name and colon,
// it does not check if the file or directories actually exist.
bool is_local_path(std::string_view path);

} // namespace MetaModule::Filesystem
