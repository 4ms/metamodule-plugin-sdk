#pragma once
#include "CoreModules/elements/elements.hh"
#include <array>
#include <cstdint>
#include <numeric>
#include <optional>
#include <span>

namespace ElementCount
{

struct Counts {
	size_t num_params = 0;
	size_t num_lights = 0;
	size_t num_inputs = 0;
	size_t num_outputs = 0;

	constexpr Counts operator+(const Counts rhs) {
		return {num_params + rhs.num_params,
				num_lights + rhs.num_lights,
				num_inputs + rhs.num_inputs,
				num_outputs + rhs.num_outputs};
	}

	bool operator==(const Counts &rhs) const = default;
};

struct Indices {
	uint8_t param_idx = 0;
	uint8_t light_idx = 0;
	uint8_t input_idx = 0;
	uint8_t output_idx = 0;

	static constexpr uint8_t NoElementMarker = 0xFF;

	// Indices + Counts -> Indices
	constexpr Indices operator+(const Counts rhs) {
		return {static_cast<uint8_t>(param_idx + rhs.num_params),
				static_cast<uint8_t>(light_idx + rhs.num_lights),
				static_cast<uint8_t>(input_idx + rhs.num_inputs),
				static_cast<uint8_t>(output_idx + rhs.num_outputs)};
	}

	bool operator==(const Indices &rhs) const = default;
};

constexpr bool operator==(MetaModule::BaseElement a, MetaModule::BaseElement b) {
	// We assume all elements have distinct coordinates and/or names
	// FIXME: This will fail if two elements have different type but identical elements
	return a.x_mm == b.x_mm && a.y_mm == b.y_mm && a.short_name == b.short_name && a.long_name == b.long_name;
}

constexpr Counts count(auto e) {
	return Counts{e.NumParams, e.NumLights, e.NumInputs, e.NumOutputs};
}

constexpr Counts count(const MetaModule::Element &element) {
	return std::visit([](auto e) { return count(e); }, element);
}

template<typename Info>
constexpr Counts count() {
	return std::accumulate(Info::Elements.begin(), Info::Elements.end(), Counts{}, [](auto total, auto element) {
		return total + count(element);
	});
}

template<typename Info>
constexpr std::optional<Indices> get_indices(const MetaModule::BaseElement &element) {
	Indices idx{};

	for (auto el : Info::Elements) {
		Counts el_cnt = count(el);

		if (element == std::visit([](auto e) { return static_cast<MetaModule::BaseElement &>(e); }, el)) {
			return {{idx.param_idx, idx.light_idx, idx.input_idx, idx.output_idx}};
		}

		idx = idx + el_cnt;
	}
	return {};
}

// Returns an array of Indices, where element [i] is the indices for Info::Elements[i]
template<typename Info>
consteval auto get_indices() {
	std::array<Indices, Info::Elements.size()> indices{};
	Indices running_total{};

	for (unsigned i = 0; auto el : Info::Elements) {
		Counts el_cnt = count(el);
		Indices masked_total = {
			.param_idx = el_cnt.num_params > 0 ? running_total.param_idx : Indices::NoElementMarker,
			.light_idx = el_cnt.num_lights > 0 ? running_total.light_idx : Indices::NoElementMarker,
			.input_idx = el_cnt.num_inputs > 0 ? running_total.input_idx : Indices::NoElementMarker,
			.output_idx = el_cnt.num_outputs > 0 ? running_total.output_idx : Indices::NoElementMarker,
		};
		indices[i++] = masked_total;
		running_total = running_total + el_cnt;
	}

	return indices;
}

inline void get_indices(std::span<const MetaModule::Element> elements, std::span<Indices> indices) {
	Indices running_total{};

	for (unsigned i = 0; const auto &el : elements) {
		Counts el_cnt = count(el);
		Indices masked_total = {
			.param_idx = el_cnt.num_params > 0 ? running_total.param_idx : Indices::NoElementMarker,
			.light_idx = el_cnt.num_lights > 0 ? running_total.light_idx : Indices::NoElementMarker,
			.input_idx = el_cnt.num_inputs > 0 ? running_total.input_idx : Indices::NoElementMarker,
			.output_idx = el_cnt.num_outputs > 0 ? running_total.output_idx : Indices::NoElementMarker,
		};
		indices[i++] = masked_total;
		running_total = running_total + el_cnt;
	}
}

template<typename Info>
constexpr std::optional<size_t> get_element_id(const MetaModule::BaseElement &element) {
	for (unsigned i = 0; auto el : Info::Elements) {
		if (element == std::visit([](auto e) { return MetaModule::BaseElement{e}; }, el))
			return i;
		i++;
	}
	return {}; //element not found
}

// For each member of count that's 0, mark the corresponding member of indices as not being an element of that type
// See tests/element_tests.cc SUBCASE("Some indices are invalid if the type does not match")
inline Indices mark_unused_indices(Indices indices, Counts count) {
	if (count.num_params == 0)
		indices.param_idx = ElementCount::Indices::NoElementMarker;

	if (count.num_inputs == 0)
		indices.input_idx = ElementCount::Indices::NoElementMarker;

	if (count.num_outputs == 0)
		indices.output_idx = ElementCount::Indices::NoElementMarker;

	if (count.num_lights == 0)
		indices.light_idx = ElementCount::Indices::NoElementMarker;

	return indices;
}

// Returns true if at least one non-empty index matches
inline bool matched(Indices x, Indices y) {
	return (x.param_idx == y.param_idx && x.param_idx != ElementCount::Indices::NoElementMarker) ||
		   (x.input_idx == y.input_idx && x.input_idx != ElementCount::Indices::NoElementMarker) ||
		   (x.output_idx == y.output_idx && x.output_idx != ElementCount::Indices::NoElementMarker) ||
		   (x.light_idx == y.light_idx && x.light_idx != ElementCount::Indices::NoElementMarker);
}

}; // namespace ElementCount
