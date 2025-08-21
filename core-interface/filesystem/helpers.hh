#pragma once
#include <string>
#include <string_view>

namespace MetaModule::Filesystem
{

// Translate a file path from a computer to a path relative to the current patch.
// num_subdirs sets how many subdirectories to capture (currently only 0 and 1 are supported).
//
// Examples with the current patch is at "sdc:/live-set/drums.yml":
// With num_subdirs = 0:
// 		/Users/4ms/music/rack/samples/loop.wav => sdc:/live-set/loop.wav
// With num_subdirs = 1:
// 		/Users/4ms/music/rack/samples/loop.wav => sdc:/live-set/samples/loop.wav
std::string translate_path_to_patchdir(std::string_view path, unsigned num_subdirs = 0);

bool is_local_path(std::string_view path);

} // namespace MetaModule::Filesystem
