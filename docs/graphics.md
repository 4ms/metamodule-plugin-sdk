# Graphics

MetaModule uses PNG files for all graphics.
Module faceplates should be 240 pixels high (128.5mm or 5.059" = 240px, so effective DPI is 47.44).

The screen is 240px x 320px, about 144 ppi. We use 16-bit color (RGB565).
Modules are displayed 240px high when viewed full-screen and 180px high when zoomed-out.
We may add future options to zoom out to 120px when zoomed-out.

All graphics for a plugin live in the `assets/` dir in the plugin base directory.

When a plugin is built, the contents of the assets directory is copied into the 
plugin directory. Notice that the assets directory itself is not copied, 
just the contents. So if you put a file in `MyPlugin/assets/mypic.png`, then your plugin
will reference as `MyPlugin/mypic.png`.

Certain special effects like shadows, subtle gradients, thin lines will not look
good on the MetaModule screen, so keep that in mind as you create artwork.


### Converting a VCV Rack plugin

If you are converting a VCV Rack plugin, you will need to convert the SVG files
to PNGs. Typically all SVGs are kept in a `res/` directory for VCV Rack
plugins. For MetaModule plugins, the `res/` dir is omitted, but otherwise the
directory structure and file base names are kept the same. Before building your
plugin, convert all the SVGs to PNGs and put them into the `assets/` dir in
your plugin folder. 

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

