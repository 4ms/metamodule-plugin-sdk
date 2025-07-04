#pragma once
#include <ui/Scrollbar.hpp>
#include <ui/common.hpp>
#include <widget/OpaqueWidget.hpp>

namespace rack::ui
{

struct ScrollWidget : widget::OpaqueWidget {
	struct Internal;
	Internal *internal;

	widget::Widget *container;
	Scrollbar *horizontalScrollbar;
	Scrollbar *verticalScrollbar;

	math::Vec offset;
	math::Rect containerBox;
	bool hideScrollbars = false;

	ScrollWidget();
	~ScrollWidget();
	math::Vec getScrollOffset() {
		return offset;
	}
	void scrollTo(math::Rect r);
	math::Rect getContainerOffsetBound();
	math::Vec getHandleOffset();
	math::Vec getHandleSize();
	bool isScrolling();
	void draw(const DrawArgs &args) override;
	void step() override;
	void onHover(const HoverEvent &e) override;
	void onButton(const ButtonEvent &e) override;
	void onDragStart(const DragStartEvent &e) override;
	void onDragMove(const DragMoveEvent &e) override;
	void onHoverScroll(const HoverScrollEvent &e) override;
	void onHoverKey(const HoverKeyEvent &e) override;
};

} // namespace rack::ui
