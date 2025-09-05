#pragma once
#include <ui/MenuEntry.hpp>
#include <ui/common.hpp>
#include <widget/OpaqueWidget.hpp>

namespace rack::ui
{

struct Menu : widget::OpaqueWidget {
	Menu *parentMenu = nullptr;
	Menu *childMenu = nullptr;
	MenuEntry *activeEntry = nullptr;
	BNDcornerFlags cornerFlags = BND_CORNER_NONE;

	Menu();
	~Menu() override;
	void setChildMenu(Menu *menu);
	void step() override;
	void draw(const DrawArgs &args) override;
	void onHoverScroll(const HoverScrollEvent &e) override;
};

} // namespace rack::ui

