#pragma once
#include <app/CircularShadow.hpp>
#include <app/PortWidget.hpp>
#include <app/common.hpp>
#include <settings.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>

namespace rack::app
{

struct SvgPort : PortWidget {
	widget::FramebufferWidget *fb;
	CircularShadow *shadow;
	widget::SvgWidget *sw;

	SvgPort();
	void setSvg(std::shared_ptr<window::Svg> svg);

	[[deprecated]] void setSVG(std::shared_ptr<window::Svg> svg) {
		setSvg(svg);
	}
};

using SVGPort = SvgPort;

struct ThemedSvgPort : SvgPort {
	std::shared_ptr<window::Svg> lightSvg;
	std::shared_ptr<window::Svg> darkSvg;

	void setSvg(std::shared_ptr<window::Svg> lightSvg, std::shared_ptr<window::Svg> darkSvg);

	void step() override {
		SvgPort::setSvg(settings::preferDarkPanels ? darkSvg : lightSvg);
		SvgPort::step();
	}
};

} // namespace rack::app
