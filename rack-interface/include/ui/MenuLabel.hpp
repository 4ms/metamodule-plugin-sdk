#pragma once
#include <ui/MenuEntry.hpp>
#include <ui/common.hpp>

namespace rack::ui
{

struct MenuLabel : MenuEntry {
	std::string text;

	void draw(const DrawArgs &args) override;
	void step() override;
};

} // namespace rack::ui
