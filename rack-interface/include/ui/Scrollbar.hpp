#pragma once
#include <widget/OpaqueWidget.hpp>

namespace rack::ui
{

struct Scrollbar : widget::OpaqueWidget {
	struct Internal;
	Internal *internal;

	bool vertical = false;

	Scrollbar();
	~Scrollbar() override;
	void draw(const DrawArgs &args) override;
	void onButton(const ButtonEvent &e) override;
	void onDragStart(const DragStartEvent &e) override;
	void onDragEnd(const DragEndEvent &e) override;
	void onDragMove(const DragMoveEvent &e) override;
};

using ScrollBar = Scrollbar;

} // namespace rack::ui
