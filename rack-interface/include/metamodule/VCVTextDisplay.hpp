#pragma once
#include "util/colors_rgb565.hh"
#include <app/LightWidget.hpp>

namespace MetaModule
{

struct VCVTextDisplay : rack::app::LightWidget {

	// metamodule needs light ids to be assigned for each VCV text display
	int firstLightId = -1;
	std::string_view font = "";
	RGB565 color = Colors565::Grey;
};

}
