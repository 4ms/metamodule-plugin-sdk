#pragma once
#include <app/common.hpp>
#include <settings.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <widget/TransparentWidget.hpp>

namespace rack::app
{

struct PanelBorder : widget::TransparentWidget {
	void draw(const DrawArgs &args) override;
};

struct SvgPanel : widget::Widget {
	widget::FramebufferWidget *fb;
	widget::SvgWidget *sw;
	PanelBorder *panelBorder;
	std::shared_ptr<window::Svg> svg;

	SvgPanel();
	void step() override;
	void setBackground(std::shared_ptr<window::Svg> svg);
};

using SVGPanel = SvgPanel;

struct ThemedSvgPanel : SvgPanel {
	std::shared_ptr<window::Svg> lightSvg;
	std::shared_ptr<window::Svg> darkSvg;

	void setBackground(std::shared_ptr<window::Svg> lightSvg, std::shared_ptr<window::Svg> darkSvg);

	void step() override {
		SvgPanel::setBackground(settings::preferDarkPanels ? darkSvg : lightSvg);
		SvgPanel::step();
	}
};

} // namespace rack::app
