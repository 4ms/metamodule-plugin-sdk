#pragma once
#include <common.hpp>
#include <settings.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <widget/Widget.hpp>

namespace rack::app
{

struct SvgScrew : widget::Widget {
	widget::FramebufferWidget *fb;
	widget::SvgWidget *sw;

	SvgScrew();
	~SvgScrew() override;
	void setSvg(std::shared_ptr<window::Svg> svg);
};

using SVGScrew = SvgScrew;

struct ThemedSvgScrew : SvgScrew {
	std::shared_ptr<window::Svg> lightSvg;
	std::shared_ptr<window::Svg> darkSvg;

	void setSvg(std::shared_ptr<window::Svg> lightSvg, std::shared_ptr<window::Svg> darkSvg) {
		this->lightSvg = lightSvg;
		this->darkSvg = darkSvg;
		SvgScrew::setSvg(settings::preferDarkPanels ? darkSvg : lightSvg);
	}

	void step() override {
		SvgScrew::setSvg(settings::preferDarkPanels ? darkSvg : lightSvg);
		SvgScrew::step();
	}
};

} // namespace rack::app
