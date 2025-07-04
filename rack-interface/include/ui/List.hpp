#pragma once
#include <widget/OpaqueWidget.hpp>

namespace rack::ui
{

struct List : widget::OpaqueWidget {
	void step() override;
};

} // namespace rack::ui
