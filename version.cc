#include "version.hh"

namespace MetaModule
{

__attribute__((__visibility__("default"))) Version sdk_version() {
	return Version{1, 4, 0};
}

} // namespace MetaModule
