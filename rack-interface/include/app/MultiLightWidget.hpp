#pragma once
#include <app/LightWidget.hpp>
#include <app/common.hpp>

namespace rack
{
namespace app
{

/** Mixes a list of colors based on a list of brightness values */
struct MultiLightWidget : LightWidget {
	/** Colors of each value state */
	std::vector<NVGcolor> baseColors;

	int getNumColors() const;
	void addBaseColor(NVGcolor baseColor);
	/** Sets the color to a linear combination of the baseColors with the given weights */
	void setBrightnesses(const std::vector<float> &brightnesses);
};

} // namespace app
} // namespace rack
