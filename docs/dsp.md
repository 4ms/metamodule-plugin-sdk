# DSP Helpers


## Stream Resampler

See [dsp/stream_resampler.hh](../core-interface/dsp/stream_resampler.hh)

StreamResampler resamples a mono or interleaved multi-channel audio stream,
sample-by-sample (not by blocks). 

Instead of providing a block of samples like you would with a typical block
sesampler, you provide a callback function that returns a sample on demand. The
`process()` function will call your callback as many times as needed until it
can produce one audio frame of data. 

The benefit of StreamResamplers are that 
- The latency is minimal (as low as possible)
- It's equally efficient at any block size
- Less memory footprint
- Making it safe for concurrent accesses to the audio input stream is easier
  than it is for block resamplers

It never allocates memory, so it's safe for use in audio threads (even when
changing number of channels or flushing). Uses a hermetian algorithm optimzed
for MetaModule.

```c++
class StreamResampler {
public:
	StreamResampler(uint32_t num_channels = 2);

	std::pair<float, float> process_stereo(Function<float(void)> &&get_input);
	float process_mono(Function<float()> &&get_input);
	void process(Function<float(void)> &&get_input, std::span<float> output);

	void set_num_channels(unsigned num_channels);
	void set_sample_rate_in_out(uint32_t input_rate, uint32_t output_rate);
	float resample_ratio(unsigned chan) const;
	void flush();
```


#### Constructor
```c++
StreamResampler(uint32_t num_channels = 2);
```

You can specify the number of channels when constructing a StreamResampler.
By default, there will be two channels (stereo audio).

#### process_stereo()
```c++
std::pair<float, float> process_stereo(Function<float(void)> &&get_input);
```

This function returns a stereo audio frame. You provide it with a 
callback function or lambda `get_input()` which it will call
as many times as needed (if at all) to produce exactly one audio frame.

The stereo frame is returned as a pair of values.
If the stream is stereo, then the returned pair will be left, right
If the stream is mono, then the returned pair will be mono, mono
If the stream has > 2 channels, then the output will be garbage.

Example usage:

```c++
float pop_sample() {
    //... return the next input sample (left, right, left, right, etc)
}

auto [left, right] = resampler.process_stereo([this] { return pop_sample(); });
```

In the above example, if the number of channels was 1, then `left` would
equal `right`.

#### process_mono()

```c++
float process_mono(Function<float()> &&get_input);
```

This function returns a mono audio value. If the stream is not mono (1 channel)
it will return 0. Like `process_stereo()` (see above) you provide it a function
callback that returns a sample. `process_mono` will call your callback as needed
until it's able to calculate a resampled value.


#### process()

```c++
void process(Function<float(void)> &&get_input, std::span<float> output);
```

This is the general form of `process_stereo` and `process_mono`.
It can be used to process any number of channels (including stereo and mono streams).
Like the other process functions, you provide it a callback function that returns a
sample, and it will call your callback until it's calculated one resampled frame.


`get_input` parameter: a lambda or function returning the next input sample as
a float. If the stream is stereo, then the samples must be interleaved, that
is, get_input() should return the left channel input, then the right, then the
left...
get_input() will be called the minimum number of times to return exactly one
frame of audio.

`output` parameter: the resampled output frame (stereo or mono).
     - If the stream is stereo (num_channels==2) then `output[0]` will be the
       resampled left channel sample, and `output[1]` will be the right channel.
     - If the stream is mono, then `output[0]` will be the resampled
       sample


Usage:
```c++
   float output[2]; //stereo audio
   process([this] { return stream.pop_sample(); }, output);
```

...where `stream.pop_sample()` returns the next interleaved input sample


## Block Resampler

See [dsp/block_resampler.hh](../core-interface/dsp/block_resampler.hh)

Namespace: MetaModule

Public interface:

```c++
class BlockResampler {
public:
	BlockResampler(uint32_t num_channels = 2);

	size_t process(uint32_t channel_index, std::span<const float> in, std::span<float> out);
	void set_samplerate_in_out(uint32_t input_rate, uint32_t output_rate);
	void set_input_stride(uint32_t stride);
	void set_output_stride(uint32_t stride);
	float ratio(unsigned chan) const;
	void flush();
//...
```

The BlockResampler resamples a block of samples to another sample rate. It uses
a lightweight hermetian algorithm, which performs efficiently on the MetaModule
hardware.

```c++
BlockResampler(uint32_t num_channels = 2);
```

To create a BlockResampler, you need to tell it how many channels your buffers have.
The input and output buffers must be interleaved, for example for stereo the buffer 
would contain samples for the Left and Right channels ordered like this: LRLRLRLRLR...

The maximum number of channels is `BlockResampler::MAX_RESAMPLER_CHANNELS` (which is 16).

```c++
size_t process(uint32_t channel_index, std::span<const float> in, std::span<float> out);
```

`process` converts samples in the `in` buffer and stores them in the
`out` buffer. It uses the current sample rate and strides.
It will convert as many samples as possible without reading or writing past the
size of the in or out buffers.
The `channel_index` parameter sets the channel to process (0=Left, 1=Right).
The return value is the index of the next sample that should be written to `out` 
(essentially, the index of the last sample written plus the output_stride).

```c++
void set_samplerate_in_out(uint32_t input_rate, uint32_t output_rate);
float ratio() const;
```

`set_samplerate_in_out` sets the input and output sample rates. The values are not important, only the
ratio between them are. The ratio is just calculated nad stored when this
function is called, no actual processing is done. If you change the ratio, then
the internal resampling buffer will be flushed the next time you call
`process`. This will cause an audio discontinuity (glitch). 

The default ratio is 1, if none is specified. 

`ratio()` returns the sample rate ratio.

```c++
void flush();
```

This clears the internal resampling buffer. You do not need to call this
manually when changing the sample rate, it's done automatically. You might want
to call this if the audio data changes substantially (e.g. the user loads a new
waveform or changes modes). If the previous 4 resamples samples were non-zero,
then calling this will create a discontinuity in the output buffer (e.g. a
short click or glitch). On the other hand if the previous 4 resampled samples
were all 0, then there will be no glitch.

```c++
void set_input_stride(uint32_t stride);
void set_output_stride(uint32_t stride);
```

These functions set the stride of the input and output buffers. The stride is
equal to the number of channels interleaved in the buffer. For stereo, it's
always 2, and for mono it's always 1. You rarely will ever need to change the
stride.
The strides will automatically be set to the number of channel when you
construct the BlockResampler.



Example usage:

We recommend using the helper class `ResamplingInterleavedBuffer`.

## ResamplingInterleavedBuffer

This is a wrapper helper class for BlockResampler. It contains
a BlockResampler and an output buffer which is sized correctly.
The output buffer is an array, contained as a member variable.
Therefore the size of a ResamplingInterleavedBuffer is the size
of the output buffer plus the size of a BlockResampler.

```c++
template<size_t MaxChans, size_t MaxBlockSize, size_t MaxResampleRatio>
class ResamplingInterleavedBuffer {
public:
	ResamplingInterleavedBuffer() = default;

	std::span<float> process_block(unsigned num_chans, std::span<const float> input) { ... }
	void set_samplerate_in_out(uint32_t input_rate, uint32_t output_rate) { ... }
	void flush() { ... }
//...
```


```c++
// Constructor:
ResamplingInterleavedBuffer<MaxChans, MaxBlockSize, MaxResampleRatio>();
```

`MaxChans`: The maximum number of channels. Typically 1 for mono, 2 for stereo.

`MaxBlockSize`: The maximum number of input samples to process at once. You may
see efficiency improvements if this is a power of 2 (or at least a multiple of 4).

`MaxResampleRatio`: The maximum resampling ratio you intend to use. Due to rounding,
make sure this is a little more than you actually require.


```c++
std::span<float> process_block(std::span<const float> input) { ... }
```

`process_block` resamples one block of audio and returns a span to the resampled audio.
The span is points to data that is stored in the `ResamplingInterleavedBuffer`, so its
lifetime is the same as the lifetime of the resampler object.


Example Usage:

```c++
#include "dsp/block_resampler.hh"
using namespace MetaModule;

std::array<float, 32> in_buffer; // samples to be resampled: interleaved LRLRLRL

// Create a resampler
// 2 = stereo, 32 = in_buffer.size()
// 1.1f = more than required 44.1kHz => 48kHz
ResamplingInterleavedBuffer<2, 32, 1.1f> resampler; 

MyModule() {
    resampler.set_samplerate_in_out(44100, 48000); //resample 44.1kHz to 48kHz
}

void update() {

    add_sample_to_in_buffer(in_buffer);

    if (in_buffer_is_full) {
        auto out = res.process_block(in);

        for (size_t i = 0; i < out.size(); i+=2) {
            float left = out[i];
            float right = out[i+1];
            //...
        }
    }
}

```
