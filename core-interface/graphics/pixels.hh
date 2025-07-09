#pragma once

#include <cstdint>
namespace MetaModule
{

struct PixelRGBA {
	uint8_t r{};
	uint8_t g{};
	uint8_t b{};
	uint8_t a{0xFF};

	constexpr PixelRGBA() = default;

	constexpr PixelRGBA(uint8_t r, uint8_t g, uint8_t b)
		: r{r}
		, g{g}
		, b{b}
		, a{0xFF} {
	}

	constexpr PixelRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		: r{r}
		, g{g}
		, b{b}
		, a{a} {
	}

	constexpr PixelRGBA(int r, int g, int b)
		: r{uint8_t(r)}
		, g{uint8_t(g)}
		, b{uint8_t(b)}
		, a{0xFF} {
	}

	constexpr PixelRGBA(int r, int g, int b, int a)
		: r{uint8_t(r)}
		, g{uint8_t(g)}
		, b{uint8_t(b)}
		, a{uint8_t(a)} {
	}

	constexpr PixelRGBA(float r, float g, float b)
		: r{uint8_t(r * 255.f)}
		, g{uint8_t(g * 255.f)}
		, b{uint8_t(b * 255.f)}
		, a{0xFF} {
	}

	constexpr PixelRGBA(float r, float g, float b, float a)
		: r{uint8_t(r * 255.f)}
		, g{uint8_t(g * 255.f)}
		, b{uint8_t(b * 255.f)}
		, a{uint8_t(a * 255.f)} {
	}

	constexpr PixelRGBA(uint32_t raw)
		: r(raw >> 16)
		, g(raw >> 8)
		, b(raw >> 0)
		, a(raw >> 24) {
	}

	constexpr uint32_t raw() {
		return (r << 16) | (g << 8) | (b << 0) | (a << 24);
	}
};

} // namespace MetaModule
