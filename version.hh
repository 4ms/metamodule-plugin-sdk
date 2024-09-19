#pragma once
#include "util/version_tools.hh"
#include <cstdint>

namespace MetaModule
{

using Version = VersionUtil::Version;

Version sdk_version();

} // namespace MetaModule
