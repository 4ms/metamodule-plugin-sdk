#pragma once
#include <ui/MenuEntry.hpp>
#include <ui/common.hpp>

namespace rack::ui
{

struct MenuSeparator : MenuEntry {
	MenuSeparator();
	void draw(const DrawArgs &args) override;
};

} // namespace rack::ui
