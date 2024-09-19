#include "version.hh"

namespace MetaModule
{

__attribute__((__visibility__("default"))) Version sdk_version() {
	return Version{2, 0, 0};
}

} // namespace MetaModule
