# Graphics Displays

Virtual modules can have screens (or "display") which display information about the modules's state.
The MetaModule recognizes two types of displays: Text and Graphics.

Text Displays have text only. Graphics Displays can have text and/or graphics
(generally speaking: filled or outlined polygons).

This guide is about Graphics Displays.

For more information about Text Displays, or for a brief comparison of the two,
see the docs page: [Text Screens/Displays](text-displays.md)

## Compatibility

Graphics Displays require firmware v2.0 or later.

At this point, there is only a VCV Rack adaptor layer for using graphics
displays, Native Plugins cannot yet use these (however please contact me if you
want to do this!)

## Usage

Often, if you are porting a module from VCV Rack to MetaModule, you do not need to do anything to use Graphics Displays: they will "just work".

Calls to `nvg*()` functions in a widget's draw() or drawLayer() function can be used normally.

For example, to draw a line:

```c++
    // Draw line
    nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x10));
    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, 0, 0);
    nvgLineTo(args.vg, 0, 40);
    nvgStroke(args.vg);
```

To draw a filled polygon:

```c++
    nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x60));
    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, p.x - 2, p.y - 4);
    nvgLineTo(args.vg, p.x - 9, p.y - 4);
    nvgLineTo(args.vg, p.x - 13, p.y);
    nvgLineTo(args.vg, p.x - 9, p.y + 4);
    nvgLineTo(args.vg, p.x - 2, p.y + 4);
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
        nvgText(args.vg, p.x - 8, p.y + 3, "T", NULL);
    }

```

### Deep explanation of how it works

When you make calls to `addChild` in the ModuleWidget's constructor, the MetaModule will notice any widget that's not derived from a `ParamWidget`, `SvgWidget`, `ModuleLightWidget`, `PortWidget`, and a few others (`LightSlider`, `VCVLightBezel`, ...) and of course `VCVTextDisplay` (see [Text Screens/Displays](text-displays.md). 
These widgets are all marked as "dynamic". 

When the module is displayed in the ModuleView (single module page), then all the widgets marked as dynamic will be updated. 
First, the ModuleWidget's step(), draw(), and drawLayer(..., 1) functions will be called. Next, these functions will be called for the dynamic widgets, in the order that they were added to the ModuleWidget. Sub-children of these widgets will not have their children draw.

Calls to nvg* functions end up rendering to a framebuffer, which is then passed to the MetaModule's GUI engine (LVGL).


## Limits

- Sub-children of widgets are not drawn. Only top-layer widgets (that is,
  direct children of the ModuleWidget). This limitation will probably be
  removed eventually.

- Only layer 1 is drawn. The draw() function is called first, and then
  drawLayer() function is called with a layer parameter of 1.

- The framerate is variable and slow. Your module should not depend on a high
  framerate. The current v2.0-dev branch attempts to hit 20 FPS for a single
  module, but when we add support for drawing dynamic elements from multiple
  modules, this will drop quickly.

- Filled polygons that are also concave will be drawn as outlined. This is a
  solvable problem but requires us to use a different polygon rendering
  library.

- SVGs cannot be drawn. Keep in mind nanosvg != nanovg. There is no support for
  nanosvg or for drawing textures.

- There are some differences in how mulit-line text is wrapped. Some nanovg
  functions used for helping do this manually are not implemented
  (nvgTextBoxBounds(), nvgTextMetrics(), ...).

## Using fonts

TTF fonts are supported. The .ttf file must be present either in your plugin's assets or in the rack component library's assets.

These fonts already provided by the rack component library:

 - DSEG7ClassicMini-Bold.ttf
 - DSEG7ClassicMini-BoldItalic.ttf
 - DSEG7ClassicMini-Italic.ttf
 - DSEG7ClassicMini-Light.ttf
 - DSEG7ClassicMini-LightItalic.ttf
 - DSEG7ClassicMini-Regular.ttf
 - DejaVuSans.ttf
 - Nunito-Bold.ttf
 - ShareTechMono-Regular.ttf


If you use a custom font, you will need to add the .ttf file to the assets/ dir.
