#pragma once
#include "CoreModules/CoreProcessor.hh"
#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

namespace MetaModule
{

// StreamingWaveformDisplay
// ------------------------
// class to display a streaming waveform, when we only know
// the value of the waveform one sample at a time
// (as opposed to displaying a waveform which we know all the values of)
// The waveform is drawn with the oldest sample at the far left
// and newer samples progressing to the right.
// Memory usage is one float per pixels in the x-dimension
//
// Usage:
//
//   struct MyModule : CoreProcessor {
//     	StreamingWaveformDisplay waveform{100,80}; //100x80 pixel display
//
//     	MyModule() {
//     		waveform.set_wave_color(0x33, 0xFF, 0xBB); //teal
//     	}
//
//     	void update() override {
//     		float sample = get_next_sample();
//     		waveform.draw_sample(sample);
//     	}
//
//     	void show_graphic_display(int display_id, std::span<uint32_t> buf, unsigned width, lv_obj_t *canvas) override {
//     		waveform.show_graphic_display(buf, width, canvas);
//     	}
//
//     	bool draw_graphic_display(int display_id) override {
//     		return waveform.draw_graphic_display();
//     	}
//
//     	void hide_graphic_display(int display_id) override {
//     		waveform.hide_graphic_display();
//     	}
//   };

class StreamingWaveformDisplay {
public:
	// Construct the waveform with the dimensions of the graphic display in mm.
	// This must fit within the bounds of a graphic display of your module.
	StreamingWaveformDisplay(float display_width_mm, float display_height_mm);
	~StreamingWaveformDisplay();

	// Add a sample to the display
	// Depending on the x_zoom, it might not be drawn until enough new
	// samples have been received.
	void draw_sample(float sample);

	// Reset the waveform to 0's and start displaying from the left edge
	void sync();

	// Sets the "zoom" along the x-axis, by setting the number of samples
	// represented by one x coordinate of the pixel grid.
	// Typical values would be from 1 to 500.
	// For each x coordinate of the graphic display,
	// the minimum and maximum sample values of the samples represented by that
	// x coordinate will be drawn.
	void set_x_zoom(float zoom);

	// Sets the color of the waveform
	void set_wave_color(uint8_t r, uint8_t g, uint8_t b);
	void set_wave_color(std::span<const float, 3> rgb);

	// Sets the color of the bar at the bottom that the cursor travels along
	void set_bar_color(uint8_t r, uint8_t g, uint8_t b);
	void set_bar_color(std::span<const float, 3> rgb);

	// Sets the width of the cursor in unscaled pixels
	void set_cursor_width(unsigned width);

	// Sets the position of the cursor along the bar
	// 0 => At the left end
	// 1 => At the right end
	void set_cursor_position(float pos);

	// These three fuctions should be called from the similarly named CoreProcessor::*_graphic_display
	// functions in your module code. See example above
	void show_graphic_display(std::span<uint32_t> pix_buffer, unsigned width, lv_obj_t *lvgl_canvas);
	bool draw_graphic_display();
	void hide_graphic_display();

private:
	struct Internal;
	std::unique_ptr<Internal> internal;

	std::vector<std::pair<float, float>> samples;
	std::atomic<int> newest_sample = 0;

	float cursor_pos = 0;
	float cursor_width = 1;
	float bar_height = 3;

	float x_zoom = 1;
	float x_zoom_ctr = 0;

	uint8_t wave_r = 0, wave_g = 0xFF, wave_b = 0xFF;
	uint8_t bar_r = 0xF0, bar_g = 0x88, bar_b = 0x00;

	float scaling = 1;

	std::span<uint32_t> buffer;
	float oversample_max = 0;
	float oversample_min = 0;

	const float display_width;
	const float display_height;
	const float wave_height;
};
} // namespace MetaModule
