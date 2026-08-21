# Change Log for SDK API

### In progress
- Fixed `aligned_alloc()` failing to link with a missing
  `_memalign_r` symbol. The SDK now forwards it to `memalign`, which is already
  an API symbol in firmware.
  

### v2.3.0
- Support for exceptions crossing the plugin-host boundary: throwing an
  exception in a plugin can be caught in firmware, and vice-versa. This allows
  firmware to respond to plugins attempting to allocate too much memory, and stop
  the patch or module gracefully rather than crashing.
- Add USB query functions (see  [System API](docs/system-api.md)):
   - System::get_usb_connection_status()
   - System::get_usb_device_name()
   - System::get_usb_midi_rx_cable() and System::get_usb_midi_tx_cable(): look
     up a MIDI port of the attached device by cable number, so a module can
     list the ports by name, filter incoming messages based on cable number, and/or
     transmit messages over a particular cable.
   - System::get_usb_midi_in_jack_info() and System::get_usb_midi_out_jack_info():
     low-level information from the device's descriptors.
- **Fixed MidiInput and MidiOutput API**: some headers were missing, making them
  unusable in v2.2.0 and v2.2.1
   - Expanded documentation: [MIDI](docs/midi.md)
   - Added MidiMessage::is_cc() (in midi/midi_message.hh header: not an API symbol)
- Added missing symbols `basename()` and `stat()` when linking a plugin.
- Implemented test plugins:
   - `tests/exceptions-test` tests OOM/bad_alloc, malloc alignment, and
     basename/stat regression tests.
   - New: `tests/usb-info-test` dumps everything the USB query API reports to
     the console.
   - New: `tests/midi-test` converts incoming MIDI to gate/pitch CV and gate/CV
     back to outgoing Note On/Off, with an LED to verify MIDI rx unpatched. Also
     demonstrates filtering based on cable number.


### v2.2.1

- C++ streams (fstream, iostream, sstream, etc) are fully supported in plugins.
- C++ exceptions (throw, catch, try, etc) are fully supported in plugins.
- Arm gcc toolchain v15.3 is now supported, in addition to v12.2/12.3
   - Toolchain v13.2, 13.3, 14.2, 14.3, and 15.2 are also supported by running
     a script to build the library archive
- All libc/libstdc++/etc sources are provided in a .a file. This makes
  compiling a plugin a little faster, but otherwise doesn't change anything for
  plugin developers.

Note: Even though support for streams and exceptions adds to the plugin's
possible functions it can use, the API has not changed since these all run
completely within the plugin. This means that throwing an exception from a
plugin cannot be caught by the host firmware -- the plugin must handle the
exception itself.

### v2.2.0

- New classes and types (header-only, no API change):
   - CoreProcessor::PolyPortBuffer class (used to transfer polyphonic cable data) 
   - CoreProcessor::MaxPolyChannels (constant value of 4: will likely be
     updated to 8 or 16 in v3.x SDK)
   - CoreProcessorPoly: derives from CoreProcessor, adding two virtual
     functions for polyphony support. Will be deprecated in v3.x when these new
     functions are merged into CoreProcessor.
   - SmartCoreProcessorPoly: Helper for CoreProcessorPoly. Will be deprecated and merged into SmartCoreProcessor in v3.x.
   - MidiInput (native plugin MIDI RX stream listener)
   - MidiOutput (native plugin MIDI TX queue)

- Rack modules set unused poly channels to 0 when reducing number of channels.

### v2.1.0

- Expanded API documentation 

- New functions in the API:
   - Gui::notify_user()
   - Audio::get_block_size()
   - System::free_memory()
   - System::total_memory()
   - System::hardware_random()/hardware_random_ready()
   - System::random()
   - System::delay_ms()
   - System::get_ticks()
   - Patch::mark_patch_modified()
   - Patch::get_volume()
   - Patch::get_path()
   - Patch::get_dir()
   - Filesystem::translate_path_to_local()
   - Filesystem::is_path_local()

- New classes in the API:
   - BlockResampler: resamples multi-channel blocks of audio
   - StreamResampler: resamples multi-channel streams of audio (i.e. single frames at a time).
   - StreamingWaveformDisplay: draws a waveform with a position bar
   - WavFileStream: handles streaming wav files from disk, interfacing with dr_wav and managing the concurrent-safe buffer.
   - dr_wav is part of the SDK (and is used by WavFileStream).

- Add Element type AltParamAction, which can be used by a module to perform an action when the value is changed

- async_thread.hh moved from CoreModules/ dir to threads/ (old location replaced with a file pointing to the new location -- will be deprecated eventually)

- Replace metamodule-core-interface, metamodule-rack-interface, and metamodule-plugin-libc submodules with
  normal files in normal directories (no submodules)

### v2.0.5

- Fix configInput, configOutput, and configLight when custom types are used
- Add helper scripts for converting artwork: `scripts/createinfo.py` and `scripts/vcv-artwork.py`

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
