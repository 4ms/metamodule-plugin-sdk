#include <cstdint>

namespace MetaModule::Audio
{

// Returns the audio block size. Possible values are
// 16, 32, 64, 128, 256, or 512
uint32_t get_block_size();

} // namespace MetaModule::Audio
