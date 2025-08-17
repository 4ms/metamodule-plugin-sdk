# Wave File Stream class

See [wav/wav_file_stream.hh](../core-interface/wav/wav_file_stream.hh)

TODO

#### Example Usage: simple streaming .wav file player

Typically you will put your incoming audio into a fifo or circular buffer (aka ring buffer)
and then tell the StreamResampler how to pop them out when it needs them.
In this example we are populating the ring buffer from the data in a file,
which is read in real-time in an AsyncThread. Therefore we need to use a
concurrency-safe ring buffer. In this case we are using the [LockFreeFifoSpsc class from cpputil](../cpputil/util/lockfree_fifo_spsc.hh).


```c++

class WavPlayer : CoreProcessor {
    StreamResampler resampler;

    LockFreeFifoSpsc<float, 16> in_buff; // adjust the size as needed, depending on disk speed etc.

    AsyncThread file_reader{this, [this] {
        // If the buffer has room, read the next audio frame 
        // from a file and put it in the buffer.
        if (in_buff.num_free() >= 2) {
            auto [left, right] = read_wav_file();
            in_buff.put(left);
            in_buff.put(right);
        }
    }};

public:
    // Constructor initializes the resampler settings and starts the file reading task
    WavPlayer() {
        resampler.set_sample_rate_in_out(44100, 48000); // resample 44.1kHz data to 48kHz
        resampler.set_num_channels(2); // stereo
        file_reader.start();
    }

    void update() override {
        // Pop the buffer as needed to output one LR pair
        // If we have a buffer under-run, for simplicity we just use 0
        auto [left, right] = resampler.process_stereo([this] { return in_buff.get().value_or(0); });

        // Set the output jacks
        setOutput<LEFT_OUT>(left);
        setOutput<RIGHT_OUT>(right);
    }

    //...
```
