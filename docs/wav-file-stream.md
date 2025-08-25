# WavFileStream class

See [wav/wav_file_stream.hh](../core-interface/wav/wav_file_stream.hh)

Typically on a desktop computer, wav files will be loaded into memory before being played.
Since memory on the MetaModule is limited to around 300MB, if the user wants to 
play more sample data than will fit in memory, then the files cannot be loaded into memory.

Even if there is enough memory to load the .wav files, it can take a long time, and users
would prefer not to wait 30 seconds to a minute or more for their patch to load.

The solution to both these issues is "streaming". This means the .wav files are
partially loaded into small buffers, and audio is played from those buffers. As
the audio playback consumes more samples, the buffer must be continually filled
from disk.

The buffer size can be controlled based on how much RAM you are willing to allocate
towards sample playback. The buffer is owned by this class, so it's quite possible
an instance of this class would be 8MB, 32MB or more.


If the RAM buffer is large enough to hold the entire sample, then at some point
the sample will be "fully buffered" and no more disk access is required to play
the sample.

The `WavFileStream` class handles a lot of the difficult details required to 
make streaming work properly. It's the engine behind the 4ms Basic Sample Player module,
so for a complete example, see that [module's source](https://github.com/4ms/metamodule-core-modules/blob/TSP/4ms/core/BWAVPCore.cc)

### Overview of usage:

To use this class, you need to have an AsyncThread periodically calling `read_frames_from_file()`
whenever more samples are needed to fill the buffer.

Then in the module's update() or process() function, you simply call `pop_sample()` to
get the next sample. If the wav file is stereo, call it twice; if it's mono, call it once.

```c++
struct WavFileStream {
	WavFileStream(size_t max_samples);

    // Buffer size
	bool resize(size_t max_samples);
	size_t max_size() const;
	size_t buffer_samples() const;
	size_t buffer_frames() const;

    // Load/unload .wav file
	bool load(std::string_view sample_path);
	void unload();

    // Reading frames (only call this from AsyncThread):
	void read_frames_from_file();
	void read_frames_from_file(int num_frames);

    // Getting samples (only call this from audio context):
	float pop_sample();

    // File Transport
	void reset_playback_to_frame(uint32_t frame_num);
	void seek_frame_in_file(uint32_t frame_num = 0);

    // Playback buffer state
	bool is_loaded() const;
	unsigned samples_available() const;
	unsigned frames_available() const;
	bool is_eof() const;
	bool is_file_error() const;
	unsigned current_playback_frame() const;
	unsigned latest_buffered_frame() const;
	uint32_t first_frame_in_buffer() const;

    // Wav file information
	float sample_seconds() const;
	bool is_stereo() const;
	unsigned total_frames() const;
	std::optional<uint32_t> wav_sample_rate() const;
```


#### Example Usage: simple streaming .wav file player

This example module streams a .wav file from disk, resamples it to whatever sample rate
the MetaModule is operating, plays it on the output jacks. The file is read in small
blocks as needed, so the sample file can start playing almost immediately regardless of 
its size.

```c++

class WavPlayer : CoreProcessor {
	WavFileStream stream{MaxSamples}; 
    StreamResampler resampler;
    std::atomic<bool> ready_to_play = false;
    static constexpr unint32_t Threshold = 2048; // min. frames to buffer before playing

    AsyncThread file_reader{this, [this] {
        // Stop reading when we read end of file
        if (stream.is_eof())
            return;

        // Read from file to try to keep the buffer full past the threshold
        if (stream.frames_available() < Threshold) {
            stream.read_frames_from_file();
        } else {
            ready_to_play = true;
        }
    }};

public:
    WavPlayer() {
        // Load sample header and initialize the stream
        stream.load("sample.wav");

        // Initialize resampler
        resampler.set_sample_rate_in_out(stream.wav_sample_rate().value_or(48000), sample_rate);
        resampler.set_num_channels(stream.is_stereo() ? 2 : 1);

        // Don't start playing until `Threshold` frames have been buffered
        ready_to_play = false;

        // Start the async file reading task
        file_reader.start();
    }

    void update() override {
        if (ready_to_play && stream.frames_available() > 0) {
            // Resample one audio frame
            auto [left, right] = resampler.process_stereo([this] { return stream.pop_sample(); });

            // Send resampled audio to output jacks (-5V to +5V range)
            setOutput<LeftOut>(left * 5.f);
            setOutput<RightOut>(right * 5.f);
        }
    }

    void set_samplerate(uint32_t samplerate) override {
        resampler.set_sample_rate_in_out(stream.wav_sample_rate().value_or(48000), samplerate);
    }

    //...
```
In the above example, MaxSamples is some number of samples we determine based
on how much memory we're willing to allocate. A value of 256*1024 is about the
minimum needed for a single sample playback but larger is safer if the disk is
slow.

`Threshold` is a value that determines how far ahead the AsyncThread should buffer samples.

#### Constructor

```c++
WavFileStream(size_t max_samples);
```

Creates a WavFileStream. The maxmimum size of the buffer is provided. 
The buffer will not actually be allocated at this time: that happens
later when the file is loaded.


```c++
```

