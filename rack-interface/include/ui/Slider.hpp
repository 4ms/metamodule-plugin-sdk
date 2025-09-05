#pragma once
#include <Quantity.hpp>
#include <widget/OpaqueWidget.hpp>

namespace rack::ui
{

struct Slider : widget::OpaqueWidget {
	Quantity *quantity = nullptr;

	Slider();
	void draw(const DrawArgs &args) override;
	void onDragStart(const DragStartEvent &e) override;
	void onDragMove(const DragMoveEvent &e) override;
	void onDragEnd(const DragEndEvent &e) override;
	void onDoubleClick(const DoubleClickEvent &e) override;
};

} // namespace rack::ui
