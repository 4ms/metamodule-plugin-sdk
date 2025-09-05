# VCV Rack Adaptor

The MetaModule plugin SDK includes an adaptor layer so that VCV Rack plugins can be
easily compiled as MetaModule plugins, with minimal changes to the code.

The [top-level README](../README.md) has an example of how you might go about
porting a VCV Rack plugin to the MetaModule (creating the CMakeLists.txt, 
creating the json meta-data files, converting SVGs to PNGs, and building).

This guide intends to explain how the VCV Rack layer works, which is useful
to know if you run into issues while porting.

If you'd like to just see the code to see how it's done, look in `rack-interface/include`.
Many of the files are the same as they are in the VCV Rack SDK, with notable changes in:
- metamodule/ (all files in this directory)
- app/ModuleWidget.hpp
- engine/Module.hpp
- engine/Port.hpp
- plugin/Model.hpp
- dsp/minblep.hpp (pre-calculate a smaller table)
- helpers.hpp (createModel() moved to metamodule/create_model.hh, and added some menu helpers)
- midi.hpp (fixed-size buffer)

## Module class

All VCV Rack modules have two classes: one that inherits from `ModuleWidget` and 
another that inherits from `Module`.

...TODO
