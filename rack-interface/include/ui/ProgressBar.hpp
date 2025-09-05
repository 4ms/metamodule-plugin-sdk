#pragma once
#include <Quantity.hpp>
#include <ui/common.hpp>
#include <widget/Widget.hpp>

namespace rack::ui
{

struct ProgressBar : widget::Widget {
	Quantity *quantity = nullptr;

	ProgressBar();
	void draw(const DrawArgs &args) override;
};

} // namespace rack::ui

