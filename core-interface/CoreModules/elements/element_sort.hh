#pragma once
#include "CoreModules/elements/element_counter.hh"
#include "CoreModules/elements/elements.hh"
#include <vector>

namespace MetaModule
{

template<typename T>
concept ElementIndexTuple = requires(T t) {
	{
		std::get<0>(t)
	} -> std::same_as<Element &>;

	{
		std::get<1>(t)
	} -> std::same_as<ElementCount::Indices &>;
};

template<ElementIndexTuple T>
void populate_sorted_elements_indices(std::vector<T> &elem_idx,
									  std::vector<MetaModule::Element> &elements,
									  std::vector<ElementCount::Indices> &indices) {

	std::sort(elem_idx.begin(), elem_idx.end(), [](auto const &a, auto const &b) {
		auto const &aidx = std::get<1>(a);
		auto const &bidx = std::get<1>(b);

		if (aidx.param_idx < bidx.param_idx)
			return true;
		else if (aidx.param_idx > bidx.param_idx)
			return false;

		else if (aidx.input_idx < bidx.input_idx)
			return true;
		else if (aidx.input_idx > bidx.input_idx)
			return false;

		else if (aidx.output_idx < bidx.output_idx)
			return true;
		else if (aidx.output_idx > bidx.output_idx)
			return false;

		else if (aidx.light_idx < bidx.light_idx)
			return true;

		else
			return false;
	});

	elements.clear();
	indices.clear();

	auto num_elems = elem_idx.size();
	elements.reserve(num_elems);
	indices.reserve(num_elems);

	for (auto &elemidx : elem_idx) {
		elements.push_back(std::get<0>(elemidx));
		indices.push_back(std::get<1>(elemidx));
	}
}

} // namespace MetaModule
