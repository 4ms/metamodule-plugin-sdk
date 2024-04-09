#pragma once
#include <cstddef>
#include <cstdint>

namespace MetaModule
{

//pixels (default 75DPI) to mm
template<size_t DPI = 75>
static constexpr float to_mm(float px) {
	constexpr float pix_per_inch = DPI;
	constexpr float mm_per_inch = 25.4f;
	float inches = px / pix_per_inch;
	return inches * mm_per_inch;
}

static constexpr float to_mm(float px, float DPI) {
	constexpr float mm_per_inch = 25.4f;
	float inches = px / DPI;
	return inches * mm_per_inch;
}

// mm: length or position in mm
// pixels_per_3U: the module height in pixels
static constexpr float mm_to_px(float mm, uint32_t pixels_per_3U) {
	constexpr float mm_per_3U = 128.5f; //standard Eurorack size
	float pixels_per_mm = pixels_per_3U / mm_per_3U;
	return mm * pixels_per_mm;
}

template<size_t DPI = 75>
static constexpr float PNGpx_to_SVGpx(float px) {
	return px / 240.f * 5.059 * DPI;
}

} // namespace MetaModule
