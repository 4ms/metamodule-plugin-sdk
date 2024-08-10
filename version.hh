#pragma once
#include <cstdint>

namespace MetaModule
{

struct Version {
	uint8_t major;
	uint8_t minor;
	uint8_t revision;

	bool is_compatible(Version other) {
		return major == other.major && minor == other.minor;
	}
};

Version sdk_version();

} // namespace MetaModule
