#include <cstdint>

__attribute__((__visibility__("default"))) uint32_t sdk_version() {
  constexpr uint32_t MAJOR = 0;
  constexpr uint32_t MINOR = 15;
  constexpr uint32_t REVISION = 0;

  constexpr uint32_t MAJOR_M = 10000;
  constexpr uint32_t MINOR_M = 100;
  constexpr uint32_t REVISION_M = 1;

  return (MAJOR * MAJOR_M) + (MINOR * MINOR_M) + (REVISION * REVISION_M);
}
