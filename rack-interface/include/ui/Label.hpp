#pragma once
#include <widget/Widget.hpp>

namespace rack::ui
{

struct Label : widget::Widget {
	enum Alignment {
		LEFT_ALIGNMENT,
		CENTER_ALIGNMENT,
		RIGHT_ALIGNMENT,
		MM_CENTER_ALIGNMENT,
		MM_RIGHT_ALIGNMENT,
	};

	std::string text;
	float fontSize;
	float lineHeight;
	NVGcolor color;
	Alignment alignment = LEFT_ALIGNMENT;

	Label();
	void draw(const DrawArgs &args) override;
};

} // namespace rack::ui
