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


#### Load/unload a file

```c++
bool load(std::string_view sample_path);
void unload();
bool is_loaded() const;
```


`load()` loads a file, given the path. The path should contain the volume, for example:
`sdc:/samples/loop84bpm.wav`. This function returns true if the file was successfully loaded
or false if not. The buffer will be allocated or resized if necessary to accomodate the new
file size. The filesystem will read the and the file will be open
until `unload()` is called. This cannot be called from the audio context.

`unload()` closes the .wav file and clears the buffer. It does
not release the memory of the buffer. This cannot be called from the audio context.

`is_loaded()` returns true if a file is loaded, false if not.

#### Buffer size

```c++
bool resize(size_t max_samples);
```

Sets the maximum size (in number of samples) of the buffer. The actual buffer
may be smaller than this, if the file is smaller. That is:

```
buffer size in samples = minimum(max_samples, samples in the file);
```

If a .wav file is already loaded, then the buffer will be resized immediately
and the content clearted. The function will return true in this case. Otherwise
(if the buffer didn't change size, or if there is no file loaded), it returns
false.

This function may allocate so it should not be called from the audio context
unless you are sure there is no file loaded.

```c++
size_t max_size() const;
size_t buffer_samples() const;
size_t buffer_frames() const;
```

These three functions return information about the buffer. They are safe
to call from any context.

`max_size()` returns the maximum number of samples in the buffer. This is the
same as the last value passed to `resize()` (or the value passed to the
constructor if `resize()` has never been called).

`buffer_samples()` and `buffer_frames()` return the actual size of the buffer,
in number of samples or number of frames, respectively. As mentioned above,
`buffer_samples()` will be equal to the smaller of `max_size()` and the number
of samples in the file.


#### Reading from disk -> writing into the buffer

```c++
void read_frames_from_file();
void read_frames_from_file(int num_frames);
```

`read_frames_from_file()` reads a block of data from the loaded file. You can
specify the number of frames to read, but if you omit that parameter it will
automatically determine the optimal value based on the wav file format (this is
recommended unless you know what you're doing).
This cannot be called from the audio context. Call this periodically from the
AsyncThread whenever the buffer is below a threshold.

This function will set the `eof` flag or `file_error` flag if it runs into
those conditions.

#### Reading out of the buffer -> playback

```c++
float pop_sample();
```

Call this from the audio context to get the next sample in the buffer.
The samples are interleaved, so if the .wav file is stereo, this will
return alternating left and right channel samples.
The range is -1 to +1 (inclusive), so scale the value accordingly.

If the buffer is empty (that is, `samples_available() == 0`: see next section)
then this will return 0 without error.

#### Transport
```c++
void reset_playback_to_frame(uint32_t frame_num);
```

Jumps the play head to a particular frame. If the frame is in the buffer,
then the read head will just jump to that position in the buffer. Otherwise
the buffer will be cleared/reset in anticipation of needing to start buffering
from this frame. Must only be called by audio thread. The async thread should
call seek_frame_in_file after this.

```c++
void seek_frame_in_file(uint32_t frame_num = 0);
```

Jumps the wav file reading head to a particular frame index in the wav file. If
the frame is alraedy in the pre-buffer, then this does nothing at all.
Otherwise it will do a file seek. The audio thread should have just called
reset_playback_to_frame() when you call this, or if you are looping playback
then call this when is_eof is true. 
If the requested frame is in the buffer, this function does nothing.
Otherwise, the read head of the file will seek to the proper position and 
the EOF flag will be cleared.

Must only be called by async filesystem thread.


#### Current state of the buffer

All the following functions are safe to call from any context.

```c++
unsigned samples_available() const;
unsigned frames_available() const;
```

Returns the number of samples or frames available for playback in the buffer.
That is, it returns the number of samples that have been written
but not yet popped. Every time you call `pop_sample()`, this goes down
by 1 until it hits 0. Every time `read_frames_from_file()` successfully 
reads from the file, this goes up by the number of samples read.


```c++
unsigned current_playback_frame() const;
unsigned latest_buffered_frame() const;
uint32_t first_frame_in_buffer() const;
```
`current_playback_frame()` is the index of the next audio frame that will
be returned by the next call to `pop_sample()`. If this equals `total_frames()`
then the entire sample has been played.

`latest_buffered_frame()` returns the frame index that was most 
recently written to the buffer.

`first_frame_in_buffer()` returns the frmae index of the oldest frame
in the buffer. If the buffer has been filled, then this value plus
the buffer size will equal `latest_buffered_frame()`.


#### State of the file

These functions are safe to call from any context.

```c++
bool is_eof() const;
```
This returns true if the file has been read to the end.

```c++
bool is_file_error() const;
```

This returns true if there was a file error while reading.
To clear the error, unload and load the file again.


#### Wav file information

These functions return information about the currently loaded file.
They are safe to call from any context.

```c++
float sample_seconds() const;
```
Returns the length of wav file in seconds, or 0 if no file is loaded.

```c++
unsigned total_frames() const;
```
Returns the length of wav file in number of audio frames, or 0 if no file is loaded.

```c++
bool is_stereo() const;
```
Returns true if the file is loaded and has 2 channels.

```c++
unsigned wav_sample_rate() const;
```
Returns the sample rate of the wav file, or 0 if no file is loaded.

