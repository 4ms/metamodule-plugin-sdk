## metamodule-core-interface

This is the interface for modules with the MetaModule core.

It provides:

- `CoreProcessor` class. All MetaModule modules must derive from this base class. See `CoreModules/CoreProcessor.hh`

- `SmartCoreProcessor` class, which derives from `CoreProcessor`. See `CoreModules/SmartCoreProcessor.hh`.
  This is useful for creating modules that have an auto-generated Info struct
  (a constexpr struct derived from ModuleInfoBase that defines all the module's
  jacks, knobs, etc).
    - There is also a helper class `CoreHelper` which provides some helper
      functions for accessing elements from an Info class safely, without
      needing to derive from SmartCoreProcessor. See
      `CoreModules/CoreHelper.hh`
    - There are also many helper functions for dealing with info structs in
      `CoreModules/elements/`


- `register_module()` function. This allows a plugin to register a module's
  info (name, elements, faceplate, etc). See `CoreModules/register_module.hh`

- `Element` variant type. A module creates an array or vector of Elements to
  define the position, name, type, etc of all of its controls, jacks, lights
  and display screens. Each subtype in the variant is a particular type of
  object you might see on a module, for example Knob, Slider, LatchingButton,
  JackInput, RgbLight, etc... See `CoreModules/elements/elements.hh` for the
  variant definition, and `CoreModules/elements/base_element.hh` for the type
  hierarchy and fields for each type.

- `AsyncThread` class. Modules can create an AsyncThread object and pass it a
  function or lambda to run in a background thread. 


