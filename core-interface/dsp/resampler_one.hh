#pragma once
#include "util/fixed_vector.hh"
#include <algorithm>
#include <cstdint>

namespace MetaModule
{

template<size_t MAX_RESAMPLER_CHANNELS = 16>
class Resampler {
public:
	Resampler(uint32_t num_channels = 1) {
		set_num_channels(num_channels);
	}

	~Resampler() = default;

	void set_num_channels(unsigned num_channels) {
		num_chans = std::clamp<size_t>(num_channels, 1u, MAX_RESAMPLER_CHANNELS);
		chans.resize(num_chans, Channel{});
	}

	// Gets one output per channel, and returns the value in output[]
	// get_input() returns float, and is only called when needed
	void process(auto get_input, std::span<float> output) {

		if (output.size() < num_chans)
			// error
			return;

		if (ratio == 1.f) {
			for (auto &out : output) {
				out = get_input();
			}
			return;
		}

		if (should_flush) {
			should_flush = false;
			frac_pos = 0.0;

			for (auto &chan : chans) {
				chan.xm1 = 0; // This causes some error when flushing, but less phase delay
				chan.x0 = get_input();
			}
			for (auto &chan : chans)
				chan.x1 = get_input();
			for (auto &chan : chans)
				chan.x2 = get_input();
		}

		while (frac_pos >= 1.f) {

			if (frac_pos >= 3.f) {
				frac_pos = frac_pos - 3.f;

				for (auto &chan : chans) {
					chan.xm1 = chan.x2;
					chan.x0 = get_input();
				}
				for (auto &chan : chans)
					chan.x1 = get_input();
				for (auto &chan : chans)
					chan.x2 = get_input();
			}

			if (frac_pos >= 2.f) {
				frac_pos = frac_pos - 2.f;

				for (auto &chan : chans) {
					chan.xm1 = chan.x1;
					chan.x0 = chan.x2;
					chan.x1 = get_input();
				}
				for (auto &chan : chans)
					chan.x2 = get_input();
			}

			if (frac_pos >= 1.f) {
				frac_pos = frac_pos - 1.f;

				for (auto &chan : chans) {
					chan.xm1 = chan.x0;
					chan.x0 = chan.x1;
					chan.x1 = chan.x2;
					chan.x2 = get_input();
				}
			}
		}

		// calculate coefficients
		for (auto i = 0u; auto &chan : chans) {
			float a = (3 * (chan.x0 - chan.x1) - chan.xm1 + chan.x2) / 2;
			float b = 2 * chan.x1 + chan.xm1 - (5 * chan.x0 + chan.x2) / 2;
			float c = (chan.x1 - chan.xm1) / 2;

			output[i] = (((a * frac_pos) + b) * frac_pos + c) * frac_pos + chan.x0;
			i++;
		}
		frac_pos += ratio;
	}

	std::pair<float, float> process_stereo(auto get_input) {
		// pre-condition: num_chans <= 2
		float output[2];
		process(get_input, output);

		// For mono samples, copy left channel to right channel
		if (num_chans == 1)
			output[1] = output[0];

		return {output[0], output[1]};
	}

	float process_mono(auto get_input) {
		float output[1];
		process(get_input, output);
		return output[0];
	}

	void set_sample_rate_in_out(uint32_t input_rate, uint32_t output_rate) {
		auto cur_ratio = (float)input_rate / (float)output_rate;
		if (ratio != cur_ratio) {
			ratio = cur_ratio;
			should_flush = true;
		}
	}

	float resample_ratio(unsigned chan) {
		return ratio;
	}

	void flush() {
		should_flush = true;
	}

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

	FixedVector<Channel, MAX_RESAMPLER_CHANNELS> chans;

	size_t num_chans = 0;
};

using ResamplerMono = Resampler<1>;

} // namespace MetaModule
