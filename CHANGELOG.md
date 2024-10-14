# Change Log for SDK API

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
