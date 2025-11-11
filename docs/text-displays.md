# Displays (Screens)

Virtual modules can have screens (or "displays") that show information about the module's state.
The MetaModule recognizes two types of displays: Text and Graphics.

Text Displays have text only. Graphics Displays can have text and/or graphics
(generally speaking: filled or outlined polygons).

If you are porting a VCV Rack module to MetaModule, then Graphic Displays often work automatically.

For more information about Graphics Displays, see: [Graphical Screens/Displays](graphic-displays.md)


## When to use Text Displays vs. Graphics Displays?

In general, Graphic Displays are more versatile and usually work automatically
with existing VCV Rack modules. So, if possible, choose Graphics Displays.

The main advantage of using a Text Display is that they are drawn more
efficiently. However, for most purposes, Graphical Displays have a good-enough
framerate and no one would notice a difference.

The other reason to use Text Displays is that Graphics Displays are not
supported in the v1.x firmware branch, so if you are targeting that branch
then you're limited to using Text Displays only. Also, there is no native plugin
API for graphics displays (yet -- I would be happy to prioritize this if
someone wants to use it!).

For more information about Graphics Displays, see: [Graphical Screens/Displays](graphic-displays.md)

The remainder of this document refers to Text Displays only.


# Text Displays



## Using the VCV Rack adaptor

To create a display using the VCV Rack adaptor interface, create a
`MetaModule::VCVTextDisplay` object (or a class that derives from this). 

Set the object's box and pos to place it on the module faceplate.
Then set the font and color fields.

You also must give each display a unique ID (similar to how each Param or Input or Output
needs a unique ID in VCV Rack). This ID must also be different than any Light's unique ID.

For full examples, see port of the Rack MIDI_CV module [here](https://github.com/4ms/metamodule/blob/v2.0-dev/firmware/vcv_ports/RackCore/MIDI_CV.cpp), or the Befaco NoisePlethora, specifically these lines:
[here](https://github.com/hemmer/Befaco/blob/85bd071a4f39ed866892ef3370fab3f72c41126f/src/NoisePlethora.cpp#L611-L621), 
[here](https://github.com/hemmer/Befaco/blob/85bd071a4f39ed866892ef3370fab3f72c41126f/src/NoisePlethora.cpp#L678-L682), 
and [here](https://github.com/hemmer/Befaco/blob/85bd071a4f39ed866892ef3370fab3f72c41126f/src/NoisePlethora.cpp#L877-L892).


A brief example:
```c++
// Most VCV Modules have something like this already:
enum LightIds {
    SOMEVCVLIGHT1,
    SOMEVCVLIGHT2,
    NUM_LIGHTS
};

// Add this:
enum DisplayIds {
    MY_TEXT_DISPLAY = NUM_LIGHTS, // DisplayIds must not overlap with LightIds
    MY_OTHER_TEXT_DISPLAY,
    MY_NUMERIC_DISPLAY
};

// VCV Rack modules that have screens ususally define their own display widget class.
// To work on MM, this class needs to derive from VCVTextDisplay, which itself derives
// from rack::LightWidget.
// You could use an #ifdef or adjust your class to work as a LightWidget.
struct MyDisplay : MetaModule::VCVTextDisplay {
    //... nothing in here usually needs to change
};

struct MyModuleWidget : rack::ModuleWidget {
    MyModuleWidget() {
        //...
        // Something like this probably is already written:
        auto display = new MyDisplay;
        display->box = Vec(...);
        display->pos = Vec(...);

        // Add this to set the font and color for MetaModule
        display->font = "Default_12";            // see fonts.hh for all built-in fonts
        display->color = Colors565::Green;       // see cpputil/util/colors_565.hh
        display->firstLightId = MY_TEXT_DISPLAY; // must be a unique ID for each display, and LIGHT

        // This probably already exists in the VCV Rack version:
        addChild(display);
}
};
```

## Native plugin example

Create an element derived from `DynamicTextDisplay` and populate its fields.

Here is one way, following the style of `NativeExample/simple_gain_elements.cc` in the
[metamodule-plugin-examples repo](https://github.com/4ms/metamodule-plugin-examples).

```c++
DynamicTextDisplay display;
// Specify the area of the display (text outside this will be clipped)
display.x_mm = 10.f;
display.y_mm = 20.f;
display.width_mm = 7.f;
display.height_mm = 20.f;
display.coords = Coords::TopLeft; // this means x_mm, y_mm is the top-left coordinate
// Specify the font and color:
display.font = "Default_12";
display.color = RGB565::Orange;
elements[i] = display;
indices[i] = {.light_idx = MY_TEXT_DISPLAY};

// Repeat this for MY_OTHER_TEXT_DISPLAY and MY_NUMERIC_DISPLAY
```

Or to follow the style of 4ms modules: 

```c++
struct MyModuleInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"MyModule"};
    using enum Coords;
    //...

    static constexpr std::array<Element, 14> Elements{{
		//...knobs, jacks, etc...
        //
        DynamicTextDisplay{{{{{20.f, 10.f, TopLeft, "Status Screen", "", 45, 17}}}, "Loading...", Font::DefaultMedium, Colors565::Orange}},
        DynamicTextDisplay{{{{{20.f, 60.f, TopLeft, "Chord Screen", "", 25, 10}}}, "", Font::DefaultSmall, Colors565::Green}},
        DynamicTextDisplay{{{{{10.f, 90.f, TopLeft, "Number screen", "", 10, 7}}}, "", Font::DefaultTiny, Colors565::White}},
}};

    enum class Elem {
        // other element names
        // 
        MY_TEXT_DISPLAY,
        MY_OTHER_TEXT_DISPLAY,
        MY_NUMERIC_DISPLAY
    };

```

## Drawing the Text

To have the text drawn, you need to provide an override for `get_display_text()` in your module class.
The GUI engine calls this function when it wants to draw a text display on your module.
It will NOT be called concurrently with the audio loop, so you don't have to worry about concurrency.
It's OK to allocate small buffers in this function, so you don't have to be strict about avoiding allocations.

Parameters: 
- `int display_id` is the unique ID of the display being requested.
- `std::span<char> text` is an output parameter: your function should put the characters to draw here.
   The `text.length()` function tells you the maximum number of characters you are allowed to write to
   the text buffer.

Return the number of bytes actually written to `text`. If you don't want to write anything, return 0.

For example:

```c++
struct MyModule : rack::Module { // for native plugins, "rack::Module" would be "CoreProcessor"

    //...

	size_t get_display_text(int display_id, std::span<char> text) override {

        // You can fill `text` in any way you want. Here's one way:
		if (display_id == MY_TEXT_DISPLAY) {
            // This is the text we want to display:
			std::string someText = myModuleGetSomeText();
            someText += "\n";
            someText += "Bank: " + std::to_string(bankNumber);

            // Make sure we don't overrun the `text` buffer:
			size_t chars_to_copy = std::min(someText.size(), text.length());

            // Copy chars from `someText` to `text`
            std::copy(someText.data(), someText.data() + chars_to_copy, text.begin());

            return chars_to_copy;
		}

        // Here's another way:
        else if (display_id == MY_OTHER_TEXT_DISPLAY) {
			int chars_written = snprintf(text.data(), text.size(), "Chord: %s", getCurrentChord().c_str());
			return (chars_written < 0) ? 0 : chars_written;
        }

        // Here's a way to write numbers to a display:
        else if (led_id == MY_NUMERIC_DISPLAY) {
			int chars_written = snprintf(text.data(), text.size(), "%03d", someValue);
			return (chars_written < 0) ? 0 : chars_written;
        }

        // Returning 0 tells the GUI engine to blank out the display
		else 
            return 0;
	}
};
```

## Colors 

You can specify an exact color in a few ways:
```c++
display->Color = RGB565(0xFF8040);
display->Color = RGB565{(uint8_t)0xFF, 0x80, 0x40}; //same as above
display->Color = RGB565{1.f, 0.5f, 0.2f}; //pretty much the same as above
display->Color = RGB565::Orange;
```

The list of pre-defined colors (like `RGB565::Orange`) is in `cpputil/util/colors_rgb565.hh`.

## Fonts


Fonts are specified by name, with a string. This can be the name of the font if it's a built-in font,
or a path to a font binary file in the LVGL .bin format.

The list of built-in fonts is in the SDK in the `fonts.hh` file. You can use either the font variable name or the string itself.
For example, the following two lines are equivalent:

```c++
display->font = MetaModule::Font::Segment7_14;
// Same as:
display->font = "Segment7Standard_14";
```

The convention for naming a font is `FontName_SZ` where `SZ` is the font size.

You can use your own fonts by creating a `.bin` font file (see below) and then passing in
the path to the font like this: 

```c++
display->font = "PluginName/FontName_12.bin";
```

To generate `font_name.bin`, load a .ttf or .woff font file into the LVGL font conversion tool.
There is an online version of this at [https://lvgl.io/tools/fontconverter](https://lvgl.io/tools/fontconverter).

For the settings:
- Name: by convention use FontName followed by an underscore and then the size: `FontName_12`
- Size: set the height in pixels (10 is about the smallest legible font size. 48 is about 25mm high on the virtual panel)
- Bpp: 4 bit-per-pixel
- Fallback: leave blank
- Output format: Binary
- Uncheck "Font compression", "Horizontal subpixel", and "Try to use glyph color info"
- Range: by default use `0x20-0x7f`, which is the normal ASCII chars. You are welcome to use any values you want here if you are displaying other characters/emojis/icons/etc
- Symbols: this is another way to input the characters to include

Click submit, and save the resulting .bin file in your assets/ dir:
```
MyPlugin/
 |___ assets/
      |___ MyFont_12.bin
      |___ rad_panel.png
      |___ large_knob.png
      |___ ...
```

To access this font (MyFont_12), you would set font like this (remember, the `assets` dir is removed when creating a plugin)

```c++
    display->font = "MyPlugin/MyFont_12.bin";
```
