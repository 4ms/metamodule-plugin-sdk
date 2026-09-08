## plugin-libc

This directory contains the pre-compiled library for libc, libstdc++, newlib,
etc that your plugin needs to be linked with. There's a separate .a file for
each arm gcc version in the `lib/` dir. There's also a header needed for
compilation (in `include/`).

There are also some headers in `glue/` and a source file (`dso_handle.c`)
needed to rebuild the libraries.


## Reason for custom-built standard libraries

Normally when you compile with arm-none-eabi-g++ or gcc, the compiler links
against the standard library archives (.a files) that are distibuted with the
toolchain.

When creating a MetaModule plugin, we cannot use these standard libraries
achives because they were not compiled in a way that they can be relocated.
That is, they were not compiled with the -fPIC flag so they can't be linked
against your plugin which MUST be compiled with -fPIC. We need this flag
because we need to have relocations generated for all symbols (which a basic
requirement for a shared object that gets loaded as a plugin).


## Building the library

You normally would never need to rebuild the library since we already include a
pre-built binary for the latest gcc toolchain (15.3), which is backwards
compatible with all c and c++ features.

However, if in some constrained situation you need to use a particular version
of the arm gcc toolchain, you can build the library for that version by using
the scripts/build-plugin-libc-autotools.sh script. This downloads the newlib
and gcc sources and builds all required libraries with the -fPIC flag so that
dynamic loading will work. This is the same script we use for building the 
included pre-built v12.3 and v15.3 libraries.

Only arm gcc toolchains versions 12.2, 12.3, 13.2, 13.2, 14.2, 14.3, 15.2, and
15.3 are supported.

The script matches whatever newlib version and configuration the toolchain you
point it at was built with, because the archive has to agree with the headers
your plugin is compiled against. That works for distro-packaged toolchains as
well as for the [ARM GNU toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
releases, with one caveat:

> **Distro-packaged toolchains**
>
> Debian/Ubuntu's `libstdc++.a` has undefined references to `getentropy`,
> `arc4random`, `symlink`, `fchmod` and friends, which newlib does not provide.
> You can check any toolchain with:
>
> ```bash
> arm-none-eabi-nm -u $(arm-none-eabi-gcc -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -print-file-name=libstdc++.a) | grep getentropy
> ```
>
> This command lists undefined symbols, so if you see `U getentropy`, that toolchain's 
> libstdc++ will not work with modules that use `std::random_device`.
>
> The script refuses to reproduce such a configuration, so use an ARM GNU
> toolchain release instead.

To have the script automatically detect the arm gcc version on your PATH and
build for that, just invoke the script with no arguments:

```bash
scripts/build-plugin-libc-autotools.sh
```

To specify a path to the toolchain you wish to use, put the path to the bin/
directory as the argument:

```bash
scripts/build-plugin-libc-autotools.sh /path/to/bin/arm-gnu-toolchain-14.2.rel1-darwin-arm64-arm-none-eabi/bin
```

To ensure that a particular version is built, specify the major version (minor
version .3 will be assumed). If the version on your PATH is not the same, then
the script will return an error.

```bash
scripts/build-plugin-libc-autotools.sh 14
```


When done, the script will print instructions:

```
Done. To test it, copy it into plugin-libc/lib/:
  cp /path/to/metamodule-plugin-sdk/plugin-libc/autotools-build/gcc14.2/libmetamodule-plugin-libc-gcc14.a /path/to/metamodule-plugin-sdk/plugin-libc/lib/

and rebuild a plugin with the gcc 14 toolchain.
```

