## Limitations

As of v1.6, plugins have the following limitation:

  - No use of stringstream, fstream, ofstream, etc. Calls to these functions must be removed.

  - C++ exceptions must be disabled. Calls to throw or try/catch must be removed.

One way to deal with these limitations is to use `#ifdef METAMODULE` around
sections that must be removed. As we add support for things like exceptions and
streams, it'll be easy to search and remove these workarounds.

To allow for these, we need to re-build our custom libstdc++/libsupc++
libraries with -fPIC as well as exceptions and streams support (HELP WANTED!).

Another limitation is regarding file access:

  - Filesystem access is limited to read access to files that were loaded with
    the plugin. That is, a plugin cannot read files on the SD Card or USB
    drive. No write access of any kind is supported.

      - v2.x has a a non-blocking (asynchronious) task support, which is
        suitable for running filesystem calls for files on the USB drive or SD card.
        card). Status: async tasks are implemented in v2.0-dev branch. FatFS
        and POSIX APIs for filesystem access to USB or SD Cards is also
        implemented, but still under development.

If you are porting from a VCV Rack plugin, there are also the following limitations:

  - No expander modules. That is, modules cannot communicate with one another.
    Modules that use an expander will always act as if the expander is not
    present.

  - No dynamic drawing using nanovg (NVG draw commands). Calling these
    functions will still compile and run, but MetaModule does not call
    Widget::draw() or drawLayer(), so they'll have no effect. This typically
    means that modules with non-text screens will just have a blank screen. 

      - In order to use a text-only screen on v1.x, follow [the Text Display
        guide](docs/text-displays.md)

      - For v2.x, graphical and text drawing via nanovg will be supported.
        You can preview the status in the v2.0-dev branch. It works by using
        an adaptor to go from nanovg to our native GUI engine. TTF fonts are
        supported for drawing text.

  - In the v1.x branch, the module's `rack::ModuleWidget` is constructed once
    when the plugin is loaded, and then destructed and never again constructed.
    In the v2.x branch, every `rack::Module` has an associated `rack::ModuleWidget`.

