#pragma once
#include <string>
#include <string_view>

namespace MetaModule::Filesystem
{

// Translate a path to a file on a computer to a path that works on the MetaModule.
// Works by appending the file name from the foreign path to the provided local path.
// Optionally, one or more parent directories from the foreign path can be appended.
// Typically the local path would be the path to the currently playing patch file.
//
// See docs/filesystem-syscalls.md for example usage and more details
std::string translate_path_to_local(std::string_view path, std::string_view local_path, unsigned num_subdirs = 0);

// Given a path, returns true if it appears to be a valid MetaModule path.
// This only checks if the path starts with a valid volume name and colon,
// it does not check if the file or directories actually exist.
bool is_local_path(std::string_view path);

} // namespace MetaModule::Filesystem
