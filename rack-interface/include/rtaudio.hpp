#pragma once
#include <common.hpp>


namespace rack {


PRIVATE void rtaudioInit();

#if defined ARCH_MAC
/** Checks if Microphone permission is blocked on Mac. */
bool rtaudioIsMicrophoneBlocked();
#endif


} // namespace rack
