#pragma once
#include "util/callable.hh"
#include "util/fixed_vector.hh"
#include <cstdint>

namespace MetaModule
{

// Resamples a mono or interleaved multi-channel audio stream, sample-by-sample (not by blocks).
// Rather than supplying a buffer or block of samples, you supply a function that returns the
// next sample.
// Never allocates, safe for use in audio threads (even when changing number of channels or
// flushing).
// Uses a hermetian algorithm optimzed for MetaModule.
class StreamResampler {
public:
	StreamResampler(uint32_t num_channels = 2);
	~StreamResampler() = default;

	// process():
	//   param: get_input: a lambda or function returning the next input sample as a float.
	//           If the stream is stereo, then the samples must be interleaved, that is,
	//           get_input() should return the left channel input, then the right, then the left...
	//
	//   param: output: the output frame (stereo or mono).
	//       - If the stream is stereo (num_channels==2) then output[0] will be the
	//         next resampled left channel sample, and output[1] will be the right channel.
	//       - If the stream is mono (num_channeles==1), then output[0] will be the resampled
	//         sample
	//
	//   Note: get_input() will be called the minimum number of times to return exactly one
	//       frame of audio.
	//
	//  Usage:
	//     float output[4]; //four-channel audio stream
	//     process([this] { return stream.pop_sample(); }, output);
	//
	//  ...where `stream.pop_sample()` returns the next interleaved input sample

	void process(Function<float(void)> &&get_input, std::span<float> output);

	// process_stereo():
	//  A helper shortcut for calling process() when you have a mono or stereo
	//  input and you need a stereo output.
	//  The stereo frame is returned as a pair of values.
	//  If the stream is mono, then right = left.
	//  If the stream has > 2 channels, then the output will be garbage.
	//  Usage:
	//     auto [left, right] = process_stereo([this] { return stream.pop_sample(); });
	//
	std::pair<float, float> process_stereo(Function<float(void)> &&get_input);

	// process_mono():
	//   A helper shortcut for calling process() when you have a mono stream.
	//   Returns a float. If the stream is not mono, this will always return 0.
	//   Usage:
	//       float out = process_mono([this] { return stream.pop_sample(); });
	float process_mono(Function<float()> &&get_input);

	// set_num_channels():
	//   Sets the number of active channels. E.g. 1 = mono, 2 = stereo, etc.
	//   Value will be clamped to 1, MAX_CHANNELS (which is 8)
	void set_num_channels(unsigned num_channels);

	// set_sample_rate_in_out():
	//    Sets the input and output sample rates. Only the ratio between the two values
	//    is important here.
	//    If the ratio changes, the stream will be flushed on the next call to process()
	void set_sample_rate_in_out(uint32_t input_rate, uint32_t output_rate);

	// resample_ratio():
	//   Returns the resampling ratio as a float
	float resample_ratio(unsigned chan) const;

	// flush():
	//   Calling this flags the stream to be flushed on the next call to process().
	//   Flushing will reset all channels, clearing the small buffer and
	//   requiring new input data to be read.
	//   Typically this is only needed to be called to perform a hard reset,
	//   e.g. loading a new sample or re-initializing the module.
	void flush();

private:
	float ratio = 1;
	bool should_flush{true};
	float frac_pos{};

	struct Channel {
		float xm1{};
		float x0{};
		float x1{};
		float x2{};
	};

	static constexpr size_t MAX_CHANNELS = 8;

	FixedVector<Channel, MAX_CHANNELS> chans;

	size_t num_chans = 0;
};

} // namespace MetaModule
