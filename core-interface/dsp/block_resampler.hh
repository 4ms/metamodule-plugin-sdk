#pragma once
#include "util/fixed_vector.hh"
#include <array>
#include <cstdint>
#include <span>

namespace MetaModule
{

class BlockResampler {
public:
	BlockResampler(uint32_t num_channels = 2);
	~BlockResampler();

	size_t process(uint32_t channel_index, std::span<const float> in, std::span<float> out);
	void set_samplerate_in_out(uint32_t input_rate, uint32_t output_rate);
	void set_input_stride(uint32_t stride);
	void set_output_stride(uint32_t stride);
	float ratio(unsigned chan) const;
	void flush();

private:
	struct Channel {
		float ratio = 1;
		bool flush{true};

		float frac_pos{};
		float xm1{};
		float x0{};
		float x1{};
		float x2{};
	};

	static constexpr size_t MAX_RESAMPLER_CHANNELS = 16;
	FixedVector<Channel, MAX_RESAMPLER_CHANNELS> chans;

	unsigned input_stride = 1;
	unsigned output_stride = 1;
	size_t num_chans = 0;
};

////////////////////////////////////////////////
// ResamplingInterleavedBuffer
//
// Helper class for resampling interleaved buffers
// Owns the output buffer.
// Calling process() returns a span to the resampled data.
//
// Usage:
//     std::array<float, 32> in; // interleaved LRLRLR...
//
//     res.set_samplerate_in_out(48000, samplerate);
//
//     auto out = res.process_block(2, in);
//     for (size_t i = 0; i < out.size(); i+=2) {
//         float left = out[i];
//         float right = out[i+1];
//         ...
//     }
//

template<size_t MaxChans, size_t MaxBlockSize, size_t MaxResampleRatio>
class ResamplingInterleavedBuffer {
public:
	ResamplingInterleavedBuffer() = default;

	std::span<float> process_block(unsigned num_chans, std::span<const float> input) {
		core.set_input_stride(num_chans);
		core.set_output_stride(num_chans);

		auto output = std::span<float>{out_buff};
		auto output_size = 0u;

		for (auto chan = 0u; chan < num_chans; chan++) {
			output_size = core.process(chan, input, output);
		}

		return output.subspan(0, output_size);
	}

	void set_samplerate_in_out(uint32_t input_rate, uint32_t output_rate) {
		core.set_samplerate_in_out(input_rate, output_rate);
	}

	void flush() {
		core.flush();
	}

private:
	BlockResampler core{MaxChans};
	std::array<float, MaxBlockSize * MaxChans * MaxResampleRatio> out_buff;
};

} // namespace MetaModule
