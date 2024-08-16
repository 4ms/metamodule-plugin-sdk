#include "version.hh"

namespace MetaModule {

__attribute__((__visibility__("default"))) Version sdk_version() {
  return Version{
      .major = 1,
      .minor = 0,
      .revision = 0,
  };
}

} // namespace MetaModule
