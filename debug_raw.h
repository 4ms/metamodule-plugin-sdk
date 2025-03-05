#pragma once
#include <stdint.h>

//C and C++

static inline void DebugPin3High() {
	volatile uint32_t *GPIOE_BSRR = (volatile uint32_t *)(0x50006018);
	*GPIOE_BSRR = (1 << 4);
}

static inline void DebugPin3Low() {
	volatile uint32_t *GPIOE_BSRR = (volatile uint32_t *)(0x50006018);
	*GPIOE_BSRR = (1 << (4 + 16));
}

static inline void DebugPin2High() {
	volatile uint32_t *GPIOG_BSRR = (volatile uint32_t *)(0x50008018);
	*GPIOG_BSRR = (1 << 11);
}

static inline void DebugPin2Low() {
	volatile uint32_t *GPIOG_BSRR = (volatile uint32_t *)(0x50008018);
	*GPIOG_BSRR = (1 << (11 + 16));
}

static inline void DebugPin1High() {
	volatile uint32_t *GPIOE_BSRR = (volatile uint32_t *)(0x50006018);
	*GPIOE_BSRR = (1 << 2);
}

static inline void DebugPin1Low() {
	volatile uint32_t *GPIOE_BSRR = (volatile uint32_t *)(0x50006018);
	*GPIOE_BSRR = (1 << (2 + 16));
}

static inline void DebugPin0High() {
	volatile uint32_t *GPIOA_BSRR = (volatile uint32_t *)(0x50002018);
	*GPIOA_BSRR = (1 << 14);
}

static inline void DebugPin0Low() {
	volatile uint32_t *GPIOA_BSRR = (volatile uint32_t *)(0x50002018);
	*GPIOA_BSRR = (1 << (14 + 16));
}

