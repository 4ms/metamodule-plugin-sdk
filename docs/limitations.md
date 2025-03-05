## Limitations

Plugins have the following limitation:

  - No use of stringstream, fstream, ofstream, etc. Calls to these functions
    must be replaced with non-stream functions (`std::to_string` or
    `std::to_chars` can often replace stringstreams, for example)

  - C++ exceptions must be disabled. Calls to throw or try/catch must be removed.

One way to deal with these limitations is to use `#ifdef METAMODULE` around
sections that must be removed. As we add support for things like exceptions and
streams, it'll be easy to search and remove these workarounds.

To allow for these, we need to re-build our custom libstdc++/libsupc++
libraries with -fPIC as well as exceptions and streams support (HELP WANTED!).

If you are porting from a VCV Rack plugin, there are also the following limitations:

  - No expander modules. That is, modules cannot communicate with one another.
    Modules that use an expander will always act as if the expander is not
    present.

  - SVGs are not loaded or rendered. You must convert to PNGs. Loading
    dynamically is not supported, the only way to display a PNG image (besides
    the faceplate) is to create an SvgWidget in the ModuleWidget constructor
    and use `addChild()`.
