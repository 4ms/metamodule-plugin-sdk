#pragma once
#include <widget/Widget.hpp>

namespace rack::ui
{

struct Tooltip : widget::Widget {
	std::string text;

	void step() override;
	void draw(const DrawArgs &args) override;
};

} // namespace rack::ui
