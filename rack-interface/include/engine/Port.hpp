#pragma once
#include <algorithm>
#include <array>
#include <common.hpp>
#include <engine/Light.hpp>
#include <numeric>

namespace rack::engine
{

static const int PORT_MAX_CHANNELS = 4;

struct Port {
	std::array<float, PORT_MAX_CHANNELS> voltages = {};
	uint8_t channels = 0;

	enum Type {
		INPUT,
		OUTPUT,
	};

	void setVoltage(float v, uint8_t channel = 0) {
		if (channel < PORT_MAX_CHANNELS)
			voltages[channel] = v;
	}

	float getVoltage(uint8_t chan = 0) const {
		return (chan < PORT_MAX_CHANNELS) ? voltages[chan] : 0.f;
	}

	float getPolyVoltage(uint8_t channel) const {
		return isMonophonic() ? getVoltage(0) : getVoltage(channel);
	}

	float getNormalVoltage(float normalVoltage, uint8_t channel = 0) const {
		return isConnected() ? getVoltage(channel) : normalVoltage;
	}

	float getNormalPolyVoltage(float normalVoltage, uint8_t channel) const {
		return isConnected() ? getPolyVoltage(channel) : normalVoltage;
	}

	float *getVoltages(uint8_t firstChannel = 0) {
		if (firstChannel < PORT_MAX_CHANNELS)
			return &voltages[firstChannel];
		else
			return &voltages[0]; // TODO: this could be a problem if caller requests a channel out of range
	}

	//UNSAFE: caller must guarentee v[] has at least channel elements
	void readVoltages(float *v) const {
		for (uint8_t c = 0; c < channels; c++) {
			v[c] = voltages[c];
		}
	}

	//UNSAFE: caller must guarentee v[] has at least channel elements
	void writeVoltages(const float *v) {
		for (uint8_t c = 0; c < channels; c++) {
			voltages[c] = v[c];
		}
	}

	void clearVoltages() {
		for (uint8_t c = 0; c < channels; c++) {
			voltages[c] = 0.f;
		}
	}

	float getVoltageSum() const {
		return std::accumulate(voltages.begin(), voltages.begin() + channels, 0.f);
	}

	float getVoltageRMS() const {
		if (channels == 0) {
			return 0.f;

		} else if (channels == 1) {
			return std::fabs(voltages[0]);

		} else {
			auto sum_squares = [](float sum, float v) {
				return sum + v * v;
			};
			return std::sqrt(std::accumulate(voltages.begin(), voltages.begin() + channels, 0.f, sum_squares));
		}
	}

	template<typename T>
	T getVoltageSimd(int firstChannel) const {
		if (firstChannel < PORT_MAX_CHANNELS)
			return T::load(&voltages[firstChannel]);
		else
			return T{0.f}; //try to handle error gracefully
	}

	template<typename T>
	T getPolyVoltageSimd(int firstChannel) const {
		return isMonophonic() ? getVoltage(0) : getVoltageSimd<T>(firstChannel);
	}

	template<typename T>
	T getNormalVoltageSimd(T normalVoltage, int firstChannel) const {
		return isConnected() ? getVoltageSimd<T>(firstChannel) : normalVoltage;
	}

	template<typename T>
	T getNormalPolyVoltageSimd(T normalVoltage, int firstChannel) const {
		return isConnected() ? getPolyVoltageSimd<T>(firstChannel) : normalVoltage;
	}

	template<typename T>
	void setVoltageSimd(T v, int firstChannel) {
		if (firstChannel < PORT_MAX_CHANNELS)
			v.store(&voltages[firstChannel]);
	}

	void setChannels(int channels) {
		// If disconnected, keep the number of channels at 0.
		if (this->channels == 0) {
			return;
		}

		// Don't allow caller to set port as disconnected
		// or out of range
		this->channels = std::clamp(channels, 1, PORT_MAX_CHANNELS);

		// VCV comment: "Set higher channel voltages to 0"
		// TODO: do this when we enable polyphony
	}

	int getChannels() const {
		return channels;
	}

	bool isConnected() const {
		return channels > 0;
	}

	bool isMonophonic() const {
		return channels == 1;
	}

	bool isPolyphonic() const {
		return channels > 1;
	}

	DEPRECATED float normalize(float normalVoltage) const {
		return getNormalVoltage(normalVoltage);
	}
};

struct Output : Port {};

struct Input : Port {};

} // namespace rack::engine

