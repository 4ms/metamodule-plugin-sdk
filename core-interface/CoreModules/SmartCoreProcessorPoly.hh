#pragma once
#include "CoreModules/CoreHelper.hh"
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/element_counter.hh"
#include "CoreModules/elements/element_state_conversion.hh"
#include <algorithm>
#include <array>
#include <optional>

namespace MetaModule
{

// Like SmartCoreProcessor, but jacks are polyphonic (up to MaxPolyChannels channels per jack).
//
// Inputs: the patch player writes the per-channel voltages and channel counts.
// numChannels<EL>() is 0 when unpatched, 1 for mono sources, 2+ for poly cables.
// getInput<EL>(chan) returns nullopt when the jack is unpatched or chan is beyond
// its channel count.
//
// Outputs: call setChannels<EL>(n) in update() to declare how many channels an
// output carries, and setOutput<EL>(val, chan) to write each channel. An unpatched
// output has 0 channels; setChannels() keeps it that way (only mark_output_patched/
// unpatched change the count to/from 0).
template<typename INFO>
class SmartCoreProcessorPoly : public CoreProcessorPoly, public CoreHelper<INFO> {
	using Elem = typename INFO::Elem;

	constexpr static auto element_num(Elem el) {
		return static_cast<std::underlying_type_t<Elem>>(el);
	}

	constexpr static auto count(Elem el) {
		return ElementCount::count(INFO::Elements[element_num(el)]);
	}

protected:
	//
	// Module Interface:
	//

	// Outputs

	template<Elem EL>
	void setOutput(float val, unsigned chan = 0) requires(count(EL).num_outputs == 1)
	{
		auto idx = index(EL).output_idx;
		if (idx < outputValues.size()) {
			if (chan < outputValues[idx].size()) {
				outputValues[idx][chan] = val;
			}
		}
	}

	template<Elem EL>
	float getOutput(unsigned chan = 0) requires(count(EL).num_outputs == 1)
	{
		auto idx = index(EL).output_idx;
		if (idx < outputValues.size() && chan < outputValues[idx].size())
			return outputValues[idx][chan];
		return 0.f;
	}

	template<Elem EL>
	unsigned numChannels() requires(count(EL).num_outputs == 1)
	{
		auto idx = index(EL).output_idx;
		if (idx < outputChannels.size())
			return outputChannels[idx];
		else
			return 0;
	}

	template<Elem EL>
	void setChannels(unsigned num_chans) requires(count(EL).num_outputs == 1)
	{
		auto idx = index(EL).output_idx;
		if (idx < outputChannels.size()) {

			// Only mark_*_patched() can change channels from 0
			if (outputChannels[idx] == 0)
				return;

			// If reducing # of channels, zero out the old ones
			for (auto i = num_chans; i < outputChannels[idx]; i++)
				outputValues[idx][i] = 0.f;

			// Only mark_*_unpatched() can change channel to 0
			outputChannels[idx] = std::clamp(num_chans, 1u, MaxPolyChannels);
		}
	}

	template<Elem EL>
	bool isPatched() requires(count(EL).num_outputs == 1)
	{
		auto idx = index(EL).output_idx;
		if (idx < outputChannels.size())
			return outputChannels[idx];
		else
			return false;
	}

	// Inputs

	template<Elem EL>
	std::optional<float> getInput(unsigned chan = 0) requires(count(EL).num_inputs == 1)
	{
		auto idx = index(EL).input_idx;
		if (idx < inputValues.size() && chan < inputChannels[idx])
			return inputValues[idx][chan];
		else
			return std::nullopt;
	}

	template<Elem EL>
	unsigned numChannels() requires(count(EL).num_inputs == 1)
	{
		auto idx = index(EL).input_idx;
		if (idx < inputChannels.size())
			return inputChannels[idx];
		else
			return 0;
	}

	template<Elem EL>
	bool isPatched() requires(count(EL).num_inputs == 1)
	{
		return numChannels<EL>() > 0;
	}

	// Params

	template<Elem EL>
	auto getState() requires(count(EL).num_params > 0)

	{
		// get back the typed element from the list of elements
		constexpr auto &elementRef = INFO::Elements[element_num(EL)];

		// reconstruct the element with its original type
		constexpr auto variantIndex = elementRef.index();
		constexpr auto specializedElement = std::get<variantIndex>(elementRef);

		// read raw value
		std::array<float, specializedElement.NumParams> rawValues;
		for (std::size_t i = 0; i < rawValues.size(); i++) {
			rawValues[i] = getParamRaw(EL, i);
		}

		// call conversion function for that type of element
		// use shortcut for special but common case of single parameter elements
		// in order to keep the conversion functions simple
		if constexpr (rawValues.size() == 1) {
			return MetaModule::StateConversion::convertState(specializedElement, rawValues[0]);
		} else {
			return MetaModule::StateConversion::convertState(specializedElement, rawValues);
		}
	}

	// LEDs
	template<Elem EL, typename VAL>
	void setLED(const VAL &value) requires(count(EL).num_lights > 0)
	{
		// get back the typed element from the list of elements
		constexpr auto &elementRef = INFO::Elements[element_num(EL)];

		// reconstruct the element with its original type
		constexpr auto variantIndex = elementRef.index();
		constexpr auto specializedElement = std::get<variantIndex>(elementRef);

		// call conversion function for that type of element
		auto rawValues = StateConversion::convertLED(specializedElement, value);

		for (std::size_t i = 0; i < rawValues.size(); i++) {
			auto idx = index(EL);
			size_t led_idx = idx.light_idx + i;
			if (led_idx < ledValues.size())
				ledValues[led_idx] = rawValues[i];
		}
	}

	// Bypass

	void handle_bypass() {
		for (auto &out : outputValues)
			out = {};

		for (auto route : INFO::bypass_routes) {
			if (route.output < outputValues.size() && route.input < inputValues.size()) {
				outputValues[route.output] = inputValues[route.input];
				if (outputChannels[route.output] > 0)
					outputChannels[route.output] = std::clamp<unsigned>(inputChannels[route.input], 1, MaxPolyChannels);
			}
		}
	}

private:
	//
	// Private Helpers:
	//

	float getParamRaw(Elem el, size_t local_index = 0) {
		auto idx = index(el);
		size_t param_id = idx.param_idx + local_index;
		return (param_id < paramValues.size()) ? paramValues[param_id] : 0.f;
	}

	constexpr static auto counts = ElementCount::count<INFO>();
	constexpr static auto indices = ElementCount::get_indices<INFO>();

	constexpr static auto index(Elem el) {
		auto el_num = element_num(el);
		return (size_t)el_num < indices.size() ? indices[el_num] : ElementCount::NoElementIndices;
	}

public:
	//
	// CoreProcessor interface:
	//

	float get_output(int output_id) const override {
		if ((size_t)output_id < outputValues.size())
			return outputValues[output_id][0];
		else
			return 0.f;
	}

	void set_input(int input_id, float val) override {
		// Mono sources write channel 0; poly sources write the buffers directly
		if ((size_t)input_id < inputValues.size())
			inputValues[input_id][0] = val;
	}

	void set_param(int param_id, float val) override {
		if ((size_t)param_id < paramValues.size()) {
			paramValues[param_id] = val;
		}
	}

	float get_param(int param_id) const override {
		if (size_t(param_id) < paramValues.size())
			return paramValues[param_id];
		else
			return 0.f;
	}

	float get_led_brightness(int led_id) const override {
		if ((size_t)led_id < ledValues.size()) {
			return ledValues[led_id];
		} else {
			return 0.0f;
		}
	}

	void mark_all_inputs_unpatched() override {
		std::fill(inputChannels.begin(), inputChannels.end(), 0);
		for (auto &in : inputValues)
			in = {};
	}

	void mark_input_unpatched(int input_id) override {
		if ((size_t)input_id < inputValues.size()) {
			inputChannels[input_id] = 0;
			inputValues[input_id] = {};
		}
	}

	void mark_input_patched(int input_id) override {
		if ((size_t)input_id < inputValues.size()) {
			// 0  -> 1 and init the voltages to zero
			// 1+ -> no change
			if (inputChannels[input_id] == 0) {
				inputChannels[input_id] = 1;
				inputValues[input_id] = {};
			}
		}
	}

	void mark_all_outputs_unpatched() override {
		std::fill(outputChannels.begin(), outputChannels.end(), 0);
	}

	void mark_output_unpatched(int output_id) override {
		if (size_t(output_id) < outputChannels.size())
			outputChannels[output_id] = 0;
	}

	void mark_output_patched(int output_id) override {
		if (size_t(output_id) < outputChannels.size()) {
			// 0  -> 1
			// 1+ -> no change
			if (outputChannels[output_id] == 0)
				outputChannels[output_id] = 1;
		}
	}

	CoreProcessor::PolyPortBuffer get_poly_input_buffer(int input_id) override {
		if ((size_t)input_id >= inputValues.size())
			return {};

		return {inputValues[input_id].data(), &inputChannels[input_id]};
	}

	CoreProcessor::PolyPortBuffer get_poly_output_buffer(int output_id) override {
		if ((size_t)output_id >= outputValues.size())
			return {};

		return {outputValues[output_id].data(), &outputChannels[output_id]};
	}

private:
	std::array<float, counts.num_params> paramValues{};

	std::array<std::array<float, MaxPolyChannels>, counts.num_inputs> inputValues{};
	std::array<uint8_t, counts.num_inputs> inputChannels{};

	std::array<std::array<float, MaxPolyChannels>, counts.num_outputs> outputValues{};
	std::array<uint8_t, counts.num_outputs> outputChannels{};

	std::array<float, counts.num_lights> ledValues{};
};

} // namespace MetaModule
