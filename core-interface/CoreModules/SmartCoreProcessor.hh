#pragma once
#include "CoreModules/CoreHelper.hh"
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/element_counter.hh"
#include "CoreModules/elements/element_state_conversion.hh"
#include <array>
#include <optional>

namespace MetaModule
{

template<typename INFO>
class SmartCoreProcessor : public CoreProcessor, public CoreHelper<INFO> {
	using Elem = typename INFO::Elem;

	constexpr static auto element_num(Elem el) {
		return static_cast<std::underlying_type_t<Elem>>(el);
	}

	constexpr static auto count(Elem el) {
		return ElementCount::count(INFO::Elements[element_num(el)]);
	}

protected:
	template<Elem EL>
	void setOutput(float val) requires(count(EL).num_outputs == 1)
	{
		auto idx = index(EL);
		if (idx.output_idx < outputValues.size())
			outputValues[idx.output_idx] = val;
	}

	template<Elem EL>
	float getOutput() requires(count(EL).num_outputs == 1)
	{
		auto idx = index(EL);
		return idx.output_idx < outputValues.size() ? outputValues[idx.output_idx] : 0.f;
	}

	template<Elem EL>
	bool isPatched() requires(count(EL).num_outputs == 1)
	{
		auto idx = index(EL);
		if (idx.output_idx < outputPatched.size())
			return outputPatched[idx.output_idx];
		else
			return false;
	}

	template<Elem EL>
	bool isPatched() requires(count(EL).num_inputs == 1)
	{
		auto idx = index(EL);
		if (idx.input_idx < inputValues.size())
			return inputValues[idx.input_idx].has_value();
		else
			return false;
	}

	template<Elem EL>
	std::optional<float> getInput() requires(count(EL).num_inputs == 1)
	{
		auto idx = index(EL);
		if (idx.input_idx < inputValues.size()) {
			return inputValues[idx.input_idx];
		} else
			return std::nullopt;
	}

	// TODO: something like this since we never have num_params > 1
	// template<Elem EL>
	// auto getState() requires(count(EL).num_params == 1)
	// {
	// 	auto raw = getParamRaw(EL);
	// 	return std::visit([&raw](auto const &el) { return StateConversion::convertState(el, raw); },
	// 					  INFO::Elements[element_num(EL)]);
	// }

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

	// TODO: make overloads for setLED, matching on various types of LEDs or requiring NumLight values
	// Each overload converts the values and directly writes setLEDRaw (or ledValues[])
	// Then we don't need convertLED
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
			setLEDRaw(EL, rawValues[i], i);
		}
	}

private:
	float getParamRaw(Elem el, size_t local_index = 0) {
		auto idx = index(el);
		size_t param_id = idx.param_idx + local_index;
		return (param_id < paramValues.size()) ? paramValues[param_id] : 0.f;
	}

	void setLEDRaw(Elem el, float val, size_t color_idx = 0) {
		auto idx = index(el);
		size_t led_idx = idx.light_idx + color_idx;
		if (led_idx < ledValues.size())
			ledValues[led_idx] = val;
	}

	constexpr static auto counts = ElementCount::count<INFO>();
	constexpr static auto indices = ElementCount::get_indices<INFO>();

	constexpr static auto index(Elem el) {
		auto el_num = element_num(el);
		return (size_t)el_num < indices.size() ? indices[el_num] : ElementCount::NoElementIndices;
	}

public:
	float get_output(int output_id) const override {
		if ((size_t)output_id < outputValues.size())
			return outputValues[output_id];
		else
			return 0.f;
	}

	void set_input(int input_id, float val) override {
		if ((size_t)input_id < inputValues.size())
			inputValues[input_id] = val;
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
		std::fill(inputValues.begin(), inputValues.end(), std::nullopt);
	}

	void mark_input_unpatched(const int input_id) override {
		if ((size_t)input_id < inputValues.size())
			inputValues[input_id].reset();
	}

	void mark_input_patched(const int input_id) override {
		if ((size_t)input_id < inputValues.size()) {
			// Marking an input patched, but not setting a voltage on the jack: assume 0V
			if (!inputValues[input_id].has_value())
				inputValues[input_id] = 0.f;
		}
	}

	void mark_all_outputs_unpatched() override {
		std::fill(outputPatched.begin(), outputPatched.end(), false);
	}

	void mark_output_unpatched(int output_id) override {
		if (size_t(output_id) < outputPatched.size())
			outputPatched[output_id] = false;
	}

	void mark_output_patched(int output_id) override {
		if (size_t(output_id) < outputPatched.size())
			outputPatched[output_id] = true;
	}

private:
	std::array<float, counts.num_params> paramValues{};
	std::array<std::optional<float>, counts.num_inputs> inputValues{0};
	std::array<float, counts.num_outputs> outputValues{};
	std::array<float, counts.num_lights> ledValues{};
	std::array<bool, counts.num_outputs> outputPatched{};
};

} // namespace MetaModule
