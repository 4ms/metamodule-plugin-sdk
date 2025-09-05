#pragma once
#include "CoreModules/elements/element_counter.hh"
#include "CoreModules/elements/elements.hh"
#include <cstdint>

namespace MetaModule::ElementIndex
{

// get_index(element, indices):
// Extracts the correct index from the indices, based on the element type.
//
// Example:
// 		uint16_t idx = get_index(info.elements[i], info.indices[i]);
//
// If info.elements[i] derives from a ParamElement, for example, then
// idx will equal info.indices[i].param_idx
//
inline uint16_t get_index(const ParamElement &, ElementCount::Indices indices) {
	return indices.param_idx;
}

inline uint16_t get_index(const LightElement &, ElementCount::Indices indices) {
	return indices.light_idx;
}

inline uint16_t get_index(const JackInput &, ElementCount::Indices indices) {
	return indices.input_idx;
}

inline uint16_t get_index(const JackOutput &, ElementCount::Indices indices) {
	return indices.output_idx;
}

inline uint16_t get_index(const BaseElement &, ElementCount::Indices) {
	return ElementCount::Indices::NoElementMarker;
}

inline uint16_t get_index(const Element &element, ElementCount::Indices indices) {
	return std::visit([=](auto const &el) { return get_index(el, indices); }, element);
}

// set_index(element, idx):
// Given the element index and element type, this will set the correct field
// in ElementCount::Indices and set all other fields to NoElementIndices marker.
// Useful for constructing an array of indices from an array of elements.
//
// Usage example:
//		auto indices = MetaModule::ElementIndex::set_index(info.element[i], idx);
//
// If `info.element[i]` derives from a ParamElement, then the returned `indices` will be:
// {.param_idx = idx, .light_idx = NoElementIndices, .input_idx = NoElementIndices, .output_idx = NoElementIndices}
//

struct SetIndex {
	uint16_t idx;

	ElementCount::Indices operator()(const ParamElement &) {
		ElementCount::Indices indices = ElementCount::NoElementIndices;
		indices.param_idx = idx;
		return indices;
	}

	inline ElementCount::Indices operator()(const LightElement &) {
		ElementCount::Indices indices = ElementCount::NoElementIndices;
		indices.light_idx = idx;
		return indices;
	}

	inline ElementCount::Indices operator()(const JackInput &) {
		ElementCount::Indices indices = ElementCount::NoElementIndices;
		indices.input_idx = idx;
		return indices;
	}

	inline ElementCount::Indices operator()(const JackOutput &) {
		ElementCount::Indices indices = ElementCount::NoElementIndices;
		indices.output_idx = idx;
		return indices;
	}

	inline ElementCount::Indices operator()(const BaseElement &) {
		return ElementCount::NoElementIndices;
	}
};

inline ElementCount::Indices set_index(const Element &element, uint8_t idx) {
	return std::visit(MetaModule::ElementIndex::SetIndex{idx}, element);
}

}; // namespace MetaModule::ElementIndex
