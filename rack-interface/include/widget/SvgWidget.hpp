#pragma once
#include <widget/Widget.hpp>

namespace rack::widget
{

/** Owns and draws a window::Svg */
struct SvgWidget : Widget {
	std::shared_ptr<window::Svg> svg;

	SvgWidget();

	void wrap();

	void setSvg(std::shared_ptr<window::Svg> svg);

	[[deprecated]] void setSVG(std::shared_ptr<window::Svg> svg) {
		setSvg(svg);
	}

	void draw(const DrawArgs &args) override;
};

[[deprecated]] typedef SvgWidget SVGWidget;

} // namespace rack::widget
