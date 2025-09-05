#pragma once
#include <ui/Button.hpp>

namespace rack::ui
{

struct RadioButton : Button {
	void draw(const DrawArgs &args) override;
	void onDragStart(const DragStartEvent &e) override;
	void onDragEnd(const DragEndEvent &e) override;
	void onDragDrop(const DragDropEvent &e) override;
};

} // namespace rack::ui
