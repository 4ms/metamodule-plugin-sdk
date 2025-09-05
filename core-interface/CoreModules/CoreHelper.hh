#pragma once
#include "CoreModules/elements/element_counter.hh"

namespace MetaModule
{

template<typename INFO>
struct CoreHelper {
	using Elem = typename INFO::Elem;

	constexpr static auto indices = ElementCount::get_indices<INFO>();

	constexpr static auto count() {
		return ElementCount::count<INFO>();
	}

	constexpr static auto element_index(Elem el) {
		return static_cast<std::underlying_type_t<Elem>>(el);
	}

	constexpr static ElementCount::Counts count(Elem el) {
		auto element_idx = element_index(el);
		return ElementCount::count(INFO::Elements[element_idx]);
	}

	constexpr static ElementCount::Indices index(Elem el) {
		return indices[element_index(el)];
	}

	template<Elem EL>
	static constexpr auto param_index() requires(count(EL).num_params > 0)
	{
		return indices[element_index(EL)].param_idx;
	}

	template<Elem EL>
	static constexpr auto param_idx = param_index<EL>();

	template<Elem EL>
	static constexpr auto input_index() requires(count(EL).num_inputs > 0)
	{
		return indices[element_index(EL)].input_idx;
	}

	template<Elem EL>
	static constexpr auto input_idx = input_index<EL>();

	template<Elem EL>
	static constexpr auto output_index() requires(count(EL).num_outputs > 0)
	{
		return indices[element_index(EL)].output_idx;
	}

	template<Elem EL>
	static constexpr auto output_idx = output_index<EL>();

	template<Elem EL>
	static constexpr auto first_light_index() requires(count(EL).num_lights > 0)
	{
		return indices[element_index(EL)].light_idx;
	}

	template<Elem EL>
	static constexpr auto light_idx = output_index<EL>();

	template<Elem EL>
	static constexpr auto display_index() requires(count(EL).num_lights > 0)
	{
		return indices[element_index(EL)].light_idx;
	}

	template<Elem EL>
	static constexpr auto display_idx = display_index<EL>();

	template<typename T>
	static constexpr auto get_as(Elem el) {
		auto idx = element_index(el);
		return std::get_if<T>(&INFO::Elements[idx]);
	}

	static constexpr auto base_element(Elem el) {
		auto idx = element_index(el);
		return MetaModule::base_element(INFO::Elements[idx]);
	}
};

// Safely copy `message` to `dst`, without exceeding the bounds of `dst`
static constexpr size_t copy_text(std::string_view message, std::span<char> dst) {
	size_t chars_to_copy = std::min(dst.size(), message.size());
	std::copy(message.data(), message.data() + chars_to_copy, dst.begin());
	return chars_to_copy;
}

} // namespace MetaModule
