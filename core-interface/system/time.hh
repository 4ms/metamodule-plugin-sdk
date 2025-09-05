#pragma once
#include <cstdint>

namespace MetaModule::System
{

// milliseconds since power on
uint32_t get_ticks();

void delay_ms(uint32_t ms);

} // namespace MetaModule::System
