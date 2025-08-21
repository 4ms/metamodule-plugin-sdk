# CoreProcessor

This document provides an overview of the CoreProcessor class.
It focuses on help for creating native plugins, also
called "Native" plugins.
When porting modules from VCV Rack, the rack-interface
layer interfaces between the VCV Rack code and the plugin SDK's API.
However, it still may be useful to read this to understand what's
going on behind the scenes.


All modules must derive from this class.

```c++
class CoreProcessor {
public:
    // These functions are required for you to implement:
    virtual void update() = 0;
    virtual void set_samplerate(float sr) = 0;
    virtual void set_param(int param_id, float val) = 0;
    virtual float get_param(int param_id) const = 0;
    virtual void set_input(int input_id, float val) = 0;
    virtual float get_output(int output_id) const = 0;

    // All functions below here are not required to implement if you aren't using them

    // For using LEDs and text displays:
    virtual float get_led_brightness(int led_id) { return 0; }
    virtual size_t get_display_text(int display_id, std::span<char> text) { return 0; }

    // Responding to jacks being plugged/unplugged
    virtual void mark_all_inputs_unpatched() {}
    virtual void mark_input_unpatched(int input_id) {}
    virtual void mark_input_patched(int input_id) {}
    virtual void mark_all_outputs_unpatched() {}
    virtual void mark_output_unpatched(int output_id) {}
    virtual void mark_output_patched(int output_id) {}

    // For loading/saving the module state in patch files:
    virtual void load_state(std::string_view state_data) {}
    virtual std::string save_state() { return ""; }

    // For graphic displays:
	virtual void show_graphic_display(int display_id, std::span<uint32_t> pix_buffer, unsigned width, lv_obj_t *lvgl_canvas) {}
	virtual bool draw_graphic_display(int display_id) { return false; }
	virtual void hide_graphic_display(int display_id) {}
};
```


## Real-time functions (called by the audio engine):

The following functions are called by the audio engine and thus have hard
real-time requirements. These must not allocate memory (no malloc,
vector.push_back(), string.resize(), etc.), and they must not make filesystem
calls.

- `void update()`: Called at sample rate on every module in the patch. The main
processing code should go here: calculate jack and light outputs based on jack
and param inputs.

- `void set_samplerate(float sr)` Called when a module is loaded and whenever the
user changes the samplerate.

- `void set_param(int param_id, float val)` Called by the audio engine to set the
value of a knob, button, switch, etc. The value `val` will be in the range
0..1. The function must check if `param_id` is a valid value, and do nothing if
it's invalid. 

- `void get_param(int param_id)` Called by the audio engine and GUI engine to get
the value of a knob, button, switch, etc. Must return 0..1. The function must
check if `param_id` is a valid value, and should return 0 if it's invalid. Note
that the value returned by `get_param` might be different than the value the
engine assigned in `set_param` if the parameter has `integral` (aka "snap")
enabled, or some parameters interact with each other (e.g. a knob selects a
pattern which toggles the state of some buttons).

- `void set_input(int input_id, float val)`:  Called by the audio engine to send
a voltage to a jack. The value passed will be between -10 and +10, which refers
to a voltage level. The function must check if input_id is a valid id, and do 
nothing if it's invalid.

- `float get_output(int output_id)`: Called by the audio engine to get the voltage
of an output jack. The value returned must be in the range -10 to +10.
If the output_id is not valid, the function must return 0.

- `void mark_input_unpatched(int input_id)`:
- `void mark_input_patched(int input_id)`:
- `void mark_output_unpatched(int output_id)`:
- `void mark_output_patched(int output_id)`: These functions are called by the
  audio engine to tell the module that one of its jacks has changed state from
  being patched to unpatched or vice-versa. This will be called only when a
  patch is first loaded, a module is added to a patch, a user physically plugs
  or unplugs a cable into a jack, or a user creates or removes a virtual cable.
  The function must check if input_id or output_id is valid, and do nothing if
  not. Some modules may choose to ignore these, other modules may wish to use
  these for normalizations or changing state depending on the patch
  configuration.

- `void mark_all_inputs_unpatched()`:
- `void mark_all_outputs_unpatched()`: These have the same effect as above but
  are only called when the module is first loaded into a patch. It's likely the
  calls to these will be followed up by calls to individual jacks to mark them
  as patched.

### Non-realtime functions (called by the GUI engine):

The following functions are never called by the audio engine and therefore do
not have real-time requirements:

- `float get_led_brightness(int led_id)`: Called by the GUI engine to get the
  brightness of a light. Must return a value from 0 to 1, where 0 if off, and 1
  is full brightness. for RGB or dual-color lights, each element of the
  multi-color light will have its own led_id, so this function should return
  the brightness of one color component. This function is called every GUI
  update frame (max every 16ms). Since there can be large numbers of LEDs in a
  patch, it's expected that this function runs quickly. So while it's
  technically OK to allocate memory or perform complex calculations in this
  function, it will give users a bad experience (slow frame rate, laggy UI) and
  is not recommended.

- `size_t get_display_text(int display_id, std::span<char> text)`: This
  function is called by the GUI engine to get text that should be displayed in
  a DynamicTextDisplay element. If you module has no DynamicTextDisplay
  elements, then it will never be called and can be omitted from the class. The
  function should check if the display_id is valid and return 0 if not.
  Otherwise, it should fill the `text` parameter with text and return the
  number of bytes written. The function must not write characters past the end
  of the text span. That is, do not write more characters than the size of the
  span (`text.size()`). The characters written must be valid characters in the
  font of the DynamicTextDisplay. 
  It's OK to allocate memory in this function (e.g. use std::string). It's not
  recommended to make system calls such as reading from the filesystem, since
  it will make for a bad user experience (slow frame rate, laggy UI response).


- `void load_state(std::string_view state_data)`: This function is called when
  a module is first loaded or when its re-initialized (via a user clicking
  "Reset" in the Module Action menu). Modules are allowed to save their state
  into the patch file. They can use whatever format they wish as long as it can
  be represented by a std::string. Typically, json or base64 encoding are used.
  Since parameter values are saved in the patch file separately, load_state
  only needs to concern itself with any other state that won't be represented
  solely by parameter values. For example, if a module has tap tempo, then the
  tempo value should be saved and loaded. If a module chooses to implement this
  function, then it should parse/decode the state_data parameter and initialize
  its state to this value. If there is a parsing error or the data is invalid,
  then the error should not be reported and instead the module should
  initialize itself to a valid state. As a special-case, if an empty string is
  passed (state_data == "") then the module should reset to default parameter
  values and state. This function may allocate memory, make system calls, or
  perform expensive calculations. The audio context will be paused while this
  function executes.

- `std::string save_state()`: This is the complement of `load_state`. When a
  user saves a patch, then the engine will call this to get a string that
  represents the state of the module.

- `show_graphic_display()`, `draw_graphic_display()`, `hide_graphic_display()`:
  The GUI engine calls these if you registered one or more
  DynamicGraphicDisplay elements. For each of these functions, the display_id
  is passed in so you can handle multiple displays on a single module.
    - `show_graphic_display` will be called when the display is first shown on
      screen, and this is where you should perform any initialization or memory
      allocation. The pixel buffer that you should draw into is also provided,
      along with the width (so you can calculate height by
      `pix_buffer.size()/width`). You must keep a pointer to this pixel buffer because
      you need to draw into it when `draw_graphic_display` is called. Note that
      the pixel buffer passed to you is a `std::span`, meaning it's a pointer and a size. 
      So copying the `std::span` is an easy and light-weight way to store this for later use.
      The last parameter is the LVGL canvas object
      associated with the pixel buffer. If you are using LVGL widgets in your
      graphic display then they should use this as their parent. Otherwise in
      most cases you should ignore this parameter.
    - `draw_graphic_display` is called each time the display needs to be redrawn.
      The user can control the maximum frame rate, and the number of displays to 
      draw plus other things will determine the actual frequency that this is called.
      Note that you do not need to redraw everything each time this is called. 
      If only one pixel changes, then just change that one value in the pixel buffer.
      If no pixels changed, reutrn `false` and the GUI will not update the screen.
      OTherwise, return `true` and the GUI will use the pixel buffer to update the screen.
    - `hide_graphic_display`: this is called when the display is no longer
      being drawn and should be de-allocated. Once this is called, the
      `draw_graphic_display` function will not be called unless another call to
      `show_graphic_display` is made first.
      

- Constructor and destructor: The audio thread will be paused (not playing)
  when these are called, so it's safe to perform memory allocations, make
  filesystem calls, or do expensive calculations. In many situations, the
  constructor is the ideal time to populate or reserve space for `std::vector`
  or other dynamic containers, or to read from support files.

- AsyncThreads: these are tasks that your main module can launch to run
  in the background. See [./async-threads.md].

- For VCV-ported modules, context menus are called by the GUI thread and thus
  are safe to make filesystem calls or memory allocations.

