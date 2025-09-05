# Graphics Displays

Virtual modules can have screens (or "display") which display information about
the modules's state. The MetaModule recognizes two types of displays: Text and
Graphics.

Text Displays have text only. Graphics Displays can have text and/or graphics
(generally speaking: filled or outlined polygons).

This guide is about Graphics Displays.

For more information about Text Displays, or for a brief comparison of the two,
see the docs page: [Text Screens/Displays](text-displays.md)

## Compatibility

Graphics Displays require firmware v2.0 or later.

## Usage

Often, if you are porting a module from VCV Rack to MetaModule, you do not need
to do anything to use Graphics Displays: they will "just work" (However, there
are some "gotchas").

Calls to `nvg*()` functions in a widget's draw() or drawLayer() function can be
in the same way they're used in VCV Rack modules.

For example, to draw a line:

```c++
    // Draw line
    nvgStrokeColor(args.vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, 0, 0);
    nvgLineTo(args.vg, 10, 40);
    nvgStroke(args.vg);
```

To draw a filled polygon:

```c++
    nvgFillColor(args.vg, nvgRGBA(0xb8, 0x80, 0x18, 0xff));
    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, 2,  - 4);
    nvgLineTo(args.vg, 9,  - 4);
    nvgLineTo(args.vg, 13,   0);
    nvgLineTo(args.vg, 9,  + 4);
    nvgLineTo(args.vg, 2,  + 4);
    nvgClosePath(args.vg);
    nvgFill(args.vg);
```

To draw text (this uses a system font provided by the rack component library):

```c++
    std::shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/ShareTechMono-Regular.ttf"));
    if (font) {
        nvgFontSize(args.vg, 9);
        nvgFontFaceId(args.vg, font->handle);
        nvgFillColor(args.vg, nvgRGBA(0x1e, 0x28, 0x2b, 0xff));
        nvgText(args.vg, 0, 0, "Hello World", NULL);
    }

```

### Deep explanation of how it works

When you make calls to `addChild` in the ModuleWidget's constructor, the
MetaModule will notice any widget that's not derived from a `ParamWidget`,
`SvgWidget`, `ModuleLightWidget`, `PortWidget`, and a few others
(`LightSlider`, `VCVLightBezel`, `VCVTextDisplay`). These widgets are all
marked as "dynamic". Their position and box size is stored.

When the module is displayed on screen and the patch is playing, then the GUI
engine will allocate a pixel buffer for all the widgets marked as dynamic. The
widget box size will be used to determine the size of the pixel buffer.

Then, on each frame, each `rack::Module` will be updated as follows:
1. First, the ModuleWidget's step() function will be called. 
2. If the ModuleWidget implements a custom draw() or drawLayer() function, then
   they will be called also. 
3. Next, it will loop through all dynamic widgets in the order that they were
   added to the ModuleWidget: 
     A. The widget's step() function will be called.
     B. The widget's draw() and drawLayer() functions will be called. 
     C. The draw() and drawLayer() functions of widget's children will be called.

Calls to nvg* functions end up rendering to the pixel buffer, which is then
checked to see if any pixels changed. If so, the GUI back-end engine (LVGL) is
told to update that section of the screen.


## Limits

- Widgets must draw themselves within their `box`. The box size for a widget
  will determine how many pixels are allocated. Any drawing outside this box
  will be clipped. VCV Rack allows a widget to draw outside of its box, so this
  is important to check if you do not see your widget being drawn properly.
    - Exception: Text is allowed to be drawn outside the widget box. This
      is a necessity for handling centered text, where the widget box in Rack
      has a top-left coordinate of the center of the text.

- Opacity for text is ignored. All text is drawn with 100% opacity. Having the
  background content partially visible behind text does not render well on a
  small 16-bit color screen. If your module uses opacity to set a particular color
  by blending it with the background color, just set that color directly at 
  full opacity.

- Only layer 1 is drawn. The draw() function is called first, and then
  drawLayer() function is called with a layer parameter of 1.

- The framerate is variable and slow. Your module should not depend on a high
  framerate. The current v2.0-dev branch attempts to hit 20 FPS for a single
  screen on a single module, but this drops quickly as multiple modules are
  shown on screen. 

- SVGs and textures cannot be drawn. Keep in mind nanosvg != nanovg. There is
  no support for nanosvg or for drawing textures yet.

- There are some differences in how multi-line text is wrapped. Some nanovg
  functions used for helping do this manually are not implemented
  (nvgTextBoxBounds(), nvgTextMetrics(), ...). In general, if you need
  multi-line text, it's recommended to either manually insert newlines,
  or use a VCVTextDisplay (that is, a dynamic text display).


## Using fonts

TTF fonts are supported. The .ttf file must be present either in your plugin's
assets or in the rack component library's assets.

The following fonts are already provided by the rack component library, and can
be accessed using `std::shared_ptr<Font> font =
APP->window->loadFont(asset::system("res/fonts/NAMEOFFONT.ttf"));`

 - DSEG7ClassicMini-Bold.ttf
 - DSEG7ClassicMini-BoldItalic.ttf
 - DSEG7ClassicMini-Italic.ttf
 - DSEG7ClassicMini-Light.ttf
 - DSEG7ClassicMini-LightItalic.ttf
 - DSEG7ClassicMini-Regular.ttf
 - DejaVuSans.ttf
 - Nunito-Bold.ttf
 - ShareTechMono-Regular.ttf


If you use a custom font, you will need to add the .ttf file to your plugin's
assets/ dir. Then, you can access it using `std::shared_ptr<Font> font =
APP->window->loadFont(asset::plugin("NAMEOFFONT.ttf"));`

Always check your font handle is valid before attempting to use the font. While
not likely, a disk or programming error could cause a font not to load, and
it's better to fail gracefully rather than crash:

```c++
std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin("NAMEOFFONT.ttf"));
if (font && font->handle >= 0) {
    // safe to use the font here
}
```

