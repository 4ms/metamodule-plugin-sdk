#include <cstdint>
#include <span>
#include <string>

namespace MetaModule::Midi
{

std::string toPrettyString(std::span<uint8_t, 3> bytes);
std::string toPrettyMultilineString(std::span<uint8_t, 3> bytes);
} // namespace MetaModule::Midi
