# Wave File Stream class

See [wav/wav_file_stream.hh](../core-interface/wav/wav_file_stream.hh)

```c++
struct WavFileStream {
	WavFileStream(size_t max_samples);

	bool resize(size_t max_samples);
	size_t max_size() const;
	size_t buffer_size() const;

	bool load(std::string_view sample_path);
	void unload();

	void read_frames_from_file();
	void read_frames_from_file(int num_frames);

	float pop_sample();

    // Playback buffer state
	bool is_loaded() const;
	unsigned samples_available() const;
	unsigned frames_available() const;
	bool is_eof() const;
	bool is_file_error() const;
	unsigned current_playback_frame() const;
	unsigned latest_buffered_frame() const;
	uint32_t first_frame_in_buffer() const;

    // File Transport
	void reset_playback_to_frame(uint32_t frame_num);
	void seek_frame_in_file(uint32_t frame_num = 0);

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

Creates a WavFileStream with a buffer that's the size

#### Constructor

```c++
```

