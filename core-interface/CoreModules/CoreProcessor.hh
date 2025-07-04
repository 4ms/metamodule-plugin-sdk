#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

typedef struct _lv_obj_t lv_obj_t;

class CoreProcessor {
public:
	CoreProcessor() = default;

	virtual void update() = 0;

	virtual void set_samplerate(float sr) = 0;
	virtual void set_param(int param_id, float val) = 0;
	virtual void set_input(int input_id, float val) = 0;

	virtual float get_output(int output_id) const = 0;
	virtual float get_led_brightness(int led_id) const {
		return 0;
	}
	virtual size_t get_display_text(int display_id, std::span<char> text) {
		return 0;
	}
	virtual float get_param(int param_id) const {
		return 0;
	}

	virtual void mark_all_inputs_unpatched() {
	}
	virtual void mark_input_unpatched(int input_id) {
	}
	virtual void mark_input_patched(int input_id) {
	}
	virtual void mark_all_outputs_unpatched() {
	}
	virtual void mark_output_unpatched(int output_id) {
	}
	virtual void mark_output_patched(int output_id) {
	}

	virtual void load_state(std::string_view state_data) {
	}
	virtual std::string save_state() {
		return "";
	}

	virtual ~CoreProcessor() = default;

	// Whether or not the module is bypassed.
	// When bypassed, update() should simply pass inputs to outputs
	// or mute outputs
	bool bypassed{false};

	uint32_t id{};

	// Initialize graphics for a display.
	// This is called by the GUI engine to inform the module that a GraphicDisplay element is now being show on screen.
	// Typically you will use this to initialize a canvas.
	//
	// This is called in the GUI context, so it's OK to allocate memory.
	//
	// Parameters:
	// display_id: the ID of the display (e.g. display_idx<MyDisplay> )
	// pix_buffer: the buffer to which you should write the pixels when `get_canvas_pixels()` is called (RGBA8888)
	// width: the dimensions of the buffer, in pixels. Note: height = pixel_buffer.size() / width
	// lvgl_canvas: an opaque pointer referring to the drawing context. Safe to ignore. Useful if you are using LVGL to draw.
	//
	virtual void
	show_graphic_display(int display_id, std::span<uint32_t> pix_buffer, unsigned width, lv_obj_t *lvgl_canvas) {
	}

	// Write pixel data to the display's pixel buffer.
	// The pixel buffer will have been previously passed to the module via show_graphic_display().
	// If you need to manually access the red, green, blue, and alpha values, use the helper class PixelRGBA in CoreModules/pixels.hh
	//
	// This is called in the GUI context.
	//
	// Return true if made changes, false if no pixels changed
	virtual bool draw_graphic_display(int display_id) {
		return false;
	}

	// De-initialize graphics for a display
	// The GUI engine calls this to inform the module that the display is now hidden.
	// Perform any clean-up here.
	//
	// This is called in the GUI context.
	//
	virtual void hide_graphic_display(int display_id) {
	}

	// common default values, OK to override or ignore
	static constexpr float CvRangeVolts = 5.0f;
	static constexpr float MaxOutputVolts = 8.0f;
};
