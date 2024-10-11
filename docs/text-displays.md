# Text Displays (Screens)

Plugin API v1.3 supports screens on modules that display text only.

Plugins have control over the font and color.
To create a display using the VCV Rack interface, create a `MetaModule::VCVTextDisplay` object
(or a class that dervives from this). See OrangeLine and CountModula projects in the
[metamodule-plugin-examples repo](https://github.com/4ms/metamodule-plugin-examples).

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

To have the text drawn, you need to provide an overridden function in your Module class like this:
```c++
struct MyModule : rack::Module {
    //...

	size_t get_display_text(int led_id, std::span<char> text) override {
        // The GUI engine calls this function, giving it an LED Id that refers
        // to the display that we want the text for.
        // The function should populate the text parameter with characters to display,
        // and return the number of bytes written.

        // Here's one way to write chars to a display:
		if (led_id == MY_TEXT_DISPLAY) {
            // This is the text we want to display:
			std::string someText = myModuleGetSomeText();

            // Make sure we don't overrun the buffer:
			size_t chars_to_copy = std::min(someText.size(), text.length());

            // Copy chars from `someText` to `text`
            std::copy(someText.data(), someText.data() + chars_to_copy, text.begin());

		}

        // Here's a way to write numbers to a display:
        else if (led_id == MY_NUMERIC_DISPLAY) {
			int chars_written = snprintf(text.data(), text.size(), "%03d", someValue);
			return chars_written < 0 ? 0: chars_written;
        }

        // Returning 0 tells the engine not to update anything
		else 
            return 0;
	}
};
```

Notice that you specify a font with a string.
A list of built-in fonts is in `metamodule-plugin-sdk/fonts.hh`.

You can specify an exact color in a few ways:
```c++
display->Color = RGB565{(uint8_t)0xFF, 0x80, 0x40};
display->Color = RGB565{1.f, 0.5f, 0.2f}; //pretty much the same as above
display->Color = RGB565::Orange;
```

### Custom fonts

You can specify a custom font by passing in the path to the font like this:
```c++
    display->font = "PluginName/font_name.bin";
```

To generate `font_name.bin`, load a .ttf or .woff font file into the LVGL font conversion tool.
There is an online version of this at (https://lvgl.io/tools/fontconverter)[https://lvgl.io/tools/fontconverter].

For the settings:
- Name: by convention use FontName followed by an underscore and then the size: `FontName_14`
- Size: set the height in pixels (10 is about the smallest legible font size. 48 is about 25mm high on the virtual panel)
- Bpp: 4 bit-per-pixel
- Fallback: leave blank
- Output format: Binary
- Uncheck Font compression, Horizontal subpixel, and Try to use glyph color info
- Range: by default use `0x20-0x7f`, which is the normal ASCII chars. You are welcome to use any values you want here if you are displaying other characters/emojis/icons/etc
- Symbols: this is another way to input the characters to include

Click submit, and save the resulting .bin file in your assets/ dir:
```
MyPlugin/
 |___ assets/
      |___ my_font.bin
      |___ rad_panel.png
      |___ large_knob.png
      |___ ...
```

To access this font (my_font), you would set font like this (remember, the `assets` dir is removed when creating a plugin`)

```c++
    display->font = "MyPlugin/my_font.bin";
```

