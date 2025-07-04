#pragma once

// Check CPU
#ifdef METAMODULE
#define ARCH_CA7 1
#elif defined __x86_64__
#define ARCH_X64 1
#elif defined __aarch64__
#define ARCH_ARM64 1
#endif

// Check OS
#ifdef METAMODULE
#define ARCH_METAMODULE 1
#elif defined _WIN32
#define ARCH_WIN 1
#elif defined __APPLE__
#define ARCH_MAC 1
#elif defined __linux__
#define ARCH_LIN 1
#endif
