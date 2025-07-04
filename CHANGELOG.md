# Change Log for SDK API

### v2.0.4

- Change KnobSnapped::pos_name type to `const char*`
  - This goes along with the API "unbreaking" from v2.0.3

### v2.0.3

- Remove position labels from Knob class, and make new class KnobSnapped
   - This fixes the unintentional API break created in v2.0.2

- Add a flag in DynamicGraphicDisplay to indicate whether the display is for the entire module or not

### v2.0.2

- Add position labels to Knob element class, to support "snapped" knobs. Max 15 labels.

- Fixed bug where Rack MIDI Message size was not always set correctly.

### v2.0.1

- Add CoreHelper helpers to get a parameter, input, output, light, or display ID more easily:
   - E.g. `unsigned my_param_id = param_idx<MyParameterName>`

- Make SmartCoreProcessor derive from CoreHelper, so it can use the above helpers automatically

- Validate plugin-mm.json file during build


### v2.0.0

- Added lots to documentation
- Tagged v2.0-dev-13.7 as v2.0.0

### API v2.0-dev (development)

#### v2.0-dev-13

- Add CoreProcessor graphics drawing virtual functions
- Fix debug_raw pins
- Add times_r symbol
- Add get_ticks() symbol
- Add register_module overloads
- Add missing symbols for CableWidgets
- Use 16-bits for element counts: allows > 256 LEDs in a module
- Add native alignment enums to rack::ui::Label


#### v2.0-dev-12.2

- Add AsyncThread symbols
- Add Midi::toPrettyMultiLineString()
- Add missing rack MIDI symbols

#### v2.0-dev-12.0, 12.1

- Heavily modified rack classes to match the API of Rack-SDK. While this adds
  no new features or change in functionality, it makes an API break from v2.0-dev-11
  because the class layouts for many classes have changed (vtables, member layout, etc).
  The reason this was done was to allow future functionality without breaking the API 
  (that is, without having to bump up the major version number and force all plugins
  to make a new release)

#### v2.0-dev-11.1
- Added libsamplerate
- Added async file dialog box browser functions
- Added more support for rack::Menu* classes
- Added dirent functions (opendir, closedir, readdir)

#### v2.0-dev-11.0

#### v2.0-dev-10.0
- Added async task support
- Added support for drawing with ttf fonts with nanovg


### API v1.6.0 

- `presets/` folder is copied into .mmplugin file by the plugin.cmake script
   - To support this, `rack::Module::paramsTo/FromJson` are implemented in
     firmware, and their symbols are added to the API.

### API v1.5.0

- Changed rack::midi::Message to not dynamically allocate, and to use an 8-bit
  frame counter.

- Changed rack::dsp::SchmittTrigger:
    - reset() sets state to 0, not to NAN.
    - Default low and high thresholds set to 0.4f and 0.6f. Previous defaults
      were 0 and 1. This gives using knobs to control buttons a much nicer UX.

- Added helper function for registering a module (in metamodule-core-interface)

- Implemented in firmware v1.5.x: Plugins can access files in their plugin dir
  via standard syscalls (fopen, fread, fclose). No change was made to the plugin
  SDK to support this, the functionality of the implementation was changed only.

### API v1.4.1

- Fixed inline implmentation of `rack::dsp::SampleRateConverter`. Now matches Rack SDK.
  This goes along with firmware v1.4.2 which implements a resampler with the same
  interface as speex (which is what `rack::dsp::SampleRateConverter` uses)


### API v1.4

- Add virtual function override `rack::engine::Module::set_samplerate(float)`.
  Previously this was implemented in the parent class as
  `CoreProcessor::set_samplerate(float)` Adding this allows the firmware to
  call overridden `rack::engine:Module::onSampleRateChange()` when the sample
  rate changes.

### API v1.3

- Add `ModuleWidget::addChild(VCVTextDisplay*)` to api symbols. This allows
  creating widgets that become DynamicTextDisplay elements when parsed by the
  VCV Module Widget adaptor.

- Add fonts.hh file (shortcuts for using fonts)

- Added `debug_raw.h` helper file, which provides functions to toggle pins on
  headers. Useful when used with a logic analyser or scope to measure timing
  of important sections of code.


### API v1.2

- Remove use of `-funsafe-math-optimizations`

- Add BypassRoutes to ModuleInfoView (effects native plugins only)

### API v1.1

- Add to api-symbols.txt: rack::APP_* symbols (rack::APP_EDITION,
  rack::APP_OS_NAME, rack::APP_VERSION, rack::APP_CPU_NAME,
  rack::APP_EDITION_NAME, rack::APP_VERSION_MAJOR). There were already present
  in the SDK, but were missing from the api-symbols file, so would generate a
  warning if compiling a plugin that uses these.

### API v1.0
 
- Initial public release
