# Graphics

VCV Rack uses SVG files for graphical assets, but MetaModule uses PNGs. So, we
need to convert all SVGs to PNGs. Typically all SVGs are kept in a `res/`
directory for VCV Rack plugins. For MetaModule plugins, the `res/` dir is
omitted, but otherwise the directory structure and file base names are kept the
same. Before building your plugin, convert all the SVGs to PNGs and put them
into the `assets/` dir in your plugin folder. This `assets/` dir is refered to
by CMakeLists in the call to `create_plugin()`, so if you want to use a
different directory, just change it there. The Cmake script will simply copy
the contents of the `assets/` dir to the plugin dir.

There is a helper script that can convert a directory of SVGs to PNGs:
`scripts/SvgToPng.py`. Running `scripts/SvgToPng.py -h` will display the help
with available options. The script requires that you have inkscape installed
and the `inkscape` executable on your PATH. Version 1.2.2 has been tested to
work.

You can use it to convert directories, one at a time (it does not recursively
scan sub-dirs). For example, if you wanted to put your knob, jack, button, etc images 
into a folder called `components`, and you wanted your faceplate graphics in a folder
called `panels` then you could do this:

```bash
cd MyPlugin

mkdir assets 
mkdir assets/panels 
mkdir assets/components

# Convert res/panels/*.svg ==>> assets/panels/*.png: 
../scripts/SvgToPng.py --input ../path/to/rack_plugins/MyPlugin/res/panels/ --output assets/panels

# Convert res/components/*.svg ==>> assets/components/*.png:
../scripts/SvgToPng.py --input ../path/to/rack_plugins/MyPlugin/res/components/ --output assets/components
```

Note that you don't have to use a separate folder for panels and components--
it's just an example. You are free to structure your graphic dirs as you like.

The script is not sophisticated: it will try to figure out the SVG's DPI but
may get it wrong. If you need to, use the `--height=240` option when
converting faceplates to force them to be 240px. You can also specify this option
for component artwork if the script is making them the wrong size by default.

If you want to disable transparency, then add the `--white` option.

Sometimes, you may wish to not use the script at all and just generate your own
PNG files that are better suited for the low-resolution screen of the MetaModule.
This will produce the best results, but of course is time-consuming.

Certain special effects like shadows, subtle gradients, thin lines will not look
good on the MetaModule screen, so keep that in mind as you convert your artwork.

The screen is 240px x 320px, about 144 ppi. We use 16-bit color (RGB565).
Modules are displayed 240px high when viewed full-screen and 180px high when zoomed-out.
We may add future options to zoom out to 120px when zoomed-out.

