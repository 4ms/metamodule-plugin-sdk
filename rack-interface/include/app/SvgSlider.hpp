#pragma once
#include <app/SliderKnob.hpp>
#include <app/common.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>

namespace rack::app
{

struct SvgSlider : app::SliderKnob {
	widget::FramebufferWidget *fb;
	widget::SvgWidget *background;
	widget::SvgWidget *handle;
	math::Vec minHandlePos, maxHandlePos;

	SvgSlider();
	~SvgSlider();

	void setBackgroundSvg(std::shared_ptr<window::Svg> svg);
	void setHandleSvg(std::shared_ptr<window::Svg> svg);
	void setHandlePos(math::Vec minHandlePos, math::Vec maxHandlePos);
	void setHandlePosCentered(math::Vec minHandlePosCentered, math::Vec maxHandlePosCentered);
	void onChange(const ChangeEvent &e) override;

	[[deprecated]] void setBackgroundSVG(std::shared_ptr<window::Svg> svg) {
		setBackgroundSvg(svg);
	}

	[[deprecated]] void setHandleSVG(std::shared_ptr<window::Svg> svg) {
		setHandleSvg(svg);
	}

	[[deprecated]] void setSVGs(std::shared_ptr<window::Svg> backgroundSvg, std::shared_ptr<window::Svg> handleSvg) {
		setBackgroundSvg(backgroundSvg);
		setHandleSvg(handleSvg);
	}
};

using SVGSlider = SvgSlider;

} // namespace rack::app
