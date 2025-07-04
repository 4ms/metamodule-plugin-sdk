#pragma once
#include "util/fixed_vector.hh"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <span>

namespace MetaModule
{

class Resampler {
public:
	Resampler(uint32_t num_channels = 2) {
		num_chans = std::clamp<size_t>(num_channels, 1u, MAX_RESAMPLER_CHANNELS);

		chans.resize(num_chans, Channel{});

		input_stride = static_cast<uint8_t>(num_chans);
		output_stride = static_cast<uint8_t>(num_chans);
	}

	~Resampler() = default;

	size_t process(uint32_t channel_index, std::span<const float> in, std::span<float> out) {
		if (channel_index >= num_chans)
			return 0;

		Channel &chan = chans[channel_index];

		auto in_size = in.size();
		auto out_size = out.size();
		in = in.subspan(channel_index);
		out = out.subspan(channel_index);

		if (chan.ratio == 1.f) {
			// copy with strides
			auto o = 0u;
			for (auto i = 0u; i < in_size && o < out_size; i += input_stride, o += output_stride) {
				out[o] = in[i];
			}
			return o;
		}

		uint32_t inpos = 0;

		auto get_next_in = [&](float &next) {
			if (inpos < in_size) {
				next = in[inpos];
				inpos += input_stride;
			}
		};

		if (chan.flush && in_size >= 3) {
			chan.flush = false;
			chan.xm1 = 0;
			get_next_in(chan.x0);
			get_next_in(chan.x1);
			get_next_in(chan.x2);
			chan.frac_pos = 0.0;
		}

		uint32_t outpos = 0;

		while (outpos < out_size && inpos < in_size) {

			// Optimize for resample rates >= 3
			if (chan.frac_pos >= 3.f && (inpos + 3) <= in_size) {
				chan.frac_pos = chan.frac_pos - 3.f;

				// shift samples back three
				// and read a new sample
				chan.xm1 = chan.x1;
				get_next_in(chan.x0);
				get_next_in(chan.x1);
				get_next_in(chan.x2);
			}

			// Optimize for resample rates >= 2
			if (chan.frac_pos >= 2.f && (inpos + 2) <= in_size) {
				chan.frac_pos = chan.frac_pos - 2.f;

				// shift samples back two
				// and read a new sample
				chan.xm1 = chan.x1;
				chan.x0 = chan.x2;
				get_next_in(chan.x1);
				get_next_in(chan.x2);
			}

			// Optimize for resample rates >= 1
			if (chan.frac_pos >= 1.f && (inpos + 1) <= in_size) {
				chan.frac_pos = chan.frac_pos - 1.f;

				// shift samples back one
				// and read a new sample
				chan.xm1 = chan.x0;
				chan.x0 = chan.x1;
				chan.x1 = chan.x2;
				get_next_in(chan.x2);
			}

			// calculate coefficients
			float a = (3 * (chan.x0 - chan.x1) - chan.xm1 + chan.x2) / 2;
			float b = 2 * chan.x1 + chan.xm1 - (5 * chan.x0 + chan.x2) / 2;
			float c = (chan.x1 - chan.xm1) / 2;

			// calculate as many fractionally placed output points as we need
			while (chan.frac_pos < 1.f && outpos < out_size) {
				out[outpos] = (((a * chan.frac_pos) + b) * chan.frac_pos + c) * chan.frac_pos + chan.x0;
				outpos += output_stride;

				chan.frac_pos += chan.ratio;
			}
		}

		return outpos - output_stride; //index of last output written
	}

	void set_sample_rate_in_out(uint32_t input_rate, uint32_t output_rate) {
		for (auto &chan : chans) {
			auto cur_ratio = (float)input_rate / (float)output_rate;

			if (chan.ratio != cur_ratio) {
				chan.ratio = (float)input_rate / (float)output_rate;
				chan.flush = true;
			}
		}
	}

	void set_input_stride(uint32_t stride) {
		input_stride = stride;
	}

	void set_output_stride(uint32_t stride) {
		output_stride = stride;
	}

	float ratio(unsigned chan) {
		return chans[chan].ratio;
	}

	void flush() {
		for (auto &chan : chans) {
			chan.flush = true;
		}
	}

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
// Helper class for resampling interleaved buffers

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
		core.set_sample_rate_in_out(input_rate, output_rate);
	}

	void flush() {
		core.flush();
	}

private:
	Resampler core{MaxChans};
	std::array<float, MaxBlockSize * MaxChans * MaxResampleRatio> out_buff;
};

} // namespace MetaModule
