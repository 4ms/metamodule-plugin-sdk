# Graphics Helpers

See [graphics/waveform_display.hh](../core-interface/graphics/waveform_display.hh)

## StreamingWaveformDisplay

This is a class to display a streaming waveform, when we only know the value of
the waveform one sample at a time (as opposed to displaying a waveform which we
know all the values of).

The waveform is drawn with the newest sample at the far right and older samples
progressing to the left.

Memory usage is about one float per pixel in the x-dimension

This uses the ThorVG library and is best suited for plugins that are not using
the Rack adaptor.

### Example Usage:

```c++
struct MyModule : CoreProcessor {
    StreamingWaveformDisplay waveform{100,80}; //100x80 pixel display

    MyModule() {
        waveform.set_wave_color(0x33, 0xFF, 0xBB); //teal
    }

    void update() override {
        float sample = get_next_sample();
        waveform.draw_sample(sample);
    }

// Boilerplate to tie the display into your module:

    void show_graphic_display(int display_id, std::span<uint32_t> buf, unsigned width, lv_obj_t *canvas) override {
        waveform.show_graphic_display(buf, width, canvas);
    }

    bool draw_graphic_display(int display_id) override {
        return waveform.draw_graphic_display();
    }

    void hide_graphic_display(int display_id) override {
        waveform.hide_graphic_display();
    }
};
```

### Interface:

TODO
