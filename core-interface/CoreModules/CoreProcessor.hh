#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

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
	// pix_buffer: the buffer to which you should write the pixels when draw_graphic_display() is called.
	//             Pixel format is ARGB8888: (a<<24)|(r<<16)|(g<<8)|b, see PixelRGBA in graphics/pixels.hh
	// width: the dimensions of the buffer, in pixels. Note: height = pixel_buffer.size() / width
	// lvgl_canvas: an opaque pointer referring to the drawing context. Safe to ignore. Useful if you are using LVGL to draw.
	//
	virtual void
	show_graphic_display(int display_id, std::span<uint32_t> pix_buffer, unsigned width, lv_obj_t *lvgl_canvas) {
	}

	// Write pixel data to the display's pixel buffer.
	// The pixel buffer will have been previously passed to the module via show_graphic_display().
	// If you need to manually access the red, green, blue, and alpha values, use the helper class PixelRGBA in graphics/pixels.hh
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

	// Context menu:
	//
	// Override get_context_menu_items() to give your module a context menu, shown in
	// the GUI when the user opens the module's options. This is the native-module
	// equivalent of a VCV Rack module's appendContextMenu(). Return an empty vector
	// (the default) for no menu.
	//
	// A single row in the module's context menu.
	struct ContextMenuItem {
		enum class Type {
			Action,	  // Clickable row; invokes context_menu_action(index)
			Checkbox, // Toggle; shows a checkmark when `checked`; invokes context_menu_action(index)
			Slider,	  // Continuous 0..1 value; edited via a popup; invokes context_menu_set_value(index, value)
			Label,	  // Non-interactive text (e.g. a heading)
			Divider,  // Horizontal separator line
		};

		Type type = Type::Action;
		std::string name;		// The row's label
		bool checked = false;	// Checkbox only: whether the checkmark is shown
		float value = 0.f;		// Slider only: current value, range 0..1
		std::string value_text; // Optional text shown after the name (e.g. a Slider's current value)
	};

	// Return the rows to show in this module's context menu.
	// Called in the GUI context each time the menu is opened or refreshed, so it is
	// fine to allocate and to reflect current state (e.g. a Checkbox's `checked`).
	virtual std::vector<ContextMenuItem> get_context_menu_items() {
		return {};
	}

	// Called when the user clicks an Action or Checkbox row. `index` is the row's
	// position in the vector returned by get_context_menu_items(). A Checkbox handler
	// should toggle the module's own state; the new state is reflected the next time
	// get_context_menu_items() is called. Called in the GUI context.
	virtual void context_menu_action(unsigned index) {
	}

	// Called repeatedly while the user adjusts a Slider row, with `value` in 0..1.
	// `index` is the row's position in the vector. Called in the GUI context.
	virtual void context_menu_set_value(unsigned index, float value) {
	}

	// Poly:
	struct PolyPortBuffer {
		float *voltages = nullptr;
		uint8_t *channels = nullptr;
	};
	static constexpr unsigned MaxPolyChannels = 4;

	// common default values, OK to override or ignore
	static constexpr float CvRangeVolts = 5.0f;
	static constexpr float MaxOutputVolts = 8.0f;
};

struct CoreProcessorPoly : public CoreProcessor {

	virtual CoreProcessor::PolyPortBuffer get_poly_input_buffer(int input_id) {
		return {};
	}

	virtual CoreProcessor::PolyPortBuffer get_poly_output_buffer(int output_id) {
		return {};
	}
};

// TODO for v3.0:
// move get_poly_*_buffer() to CoreProcessor
// [[deprecated="Use CoreProcessor instead of CoreProcessorPoly"]] using CoreProcessorPoly = CoreProcessor;
// Create GraphicDisplay virtual base class and move *_graphic_display() virtual funcs into it.
// Create free function "ModuleFactory::register_graphic_display(std::string_view slug, int display_id, std::function<std::unique_ptr<GraphicDisplay>(void)>);
