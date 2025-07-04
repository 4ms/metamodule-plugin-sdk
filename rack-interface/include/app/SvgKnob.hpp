#pragma once
#include <app/CircularShadow.hpp>
#include <app/Knob.hpp>
#include <app/common.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <widget/TransformWidget.hpp>

namespace rack::app
{

/** A knob which rotates an SVG and caches it in a framebuffer */
struct SvgKnob : Knob {
	widget::FramebufferWidget *fb;
	CircularShadow *shadow;
	widget::TransformWidget *tw;
	widget::SvgWidget *sw;

	SvgKnob();
	void setSvg(std::shared_ptr<window::Svg> svg);

	[[deprecated]] void setSVG(std::shared_ptr<window::Svg> svg) {
		setSvg(svg);
	}

	void onChange(const ChangeEvent &e) override;
};

using SVGKnob = SvgKnob;

} // namespace rack::app
