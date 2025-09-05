#pragma once
#include <list>

#include <color.hpp>
#include <common.hpp>
#include <math.hpp>
#include <weakptr.hpp>
#include <widget/event.hpp>
#include <window/Window.hpp>

namespace rack::widget
{

struct Widget {
	/** Position relative to parent and size of widget. */
	math::Rect box = math::Rect(math::Vec(), math::Vec(INFINITY, INFINITY));
	/** Automatically set when Widget is added as a child to another Widget */
	Widget *parent = nullptr;
	std::list<Widget *> children;
	bool visible = true;
	bool requestedDelete = false;

	virtual ~Widget();

	math::Rect getBox();
	void setBox(math::Rect box);

	math::Vec getPosition();
	void setPosition(math::Vec pos);

	math::Vec getSize();
	void setSize(math::Vec size);

	widget::Widget *getParent();

	bool isVisible();
	void setVisible(bool visible);

	void show() {
		setVisible(true);
	}
	void hide() {
		setVisible(false);
	}

	void requestDelete();

	virtual math::Rect getChildrenBoundingBox();
	virtual math::Rect getVisibleChildrenBoundingBox();

	bool isDescendantOf(Widget *ancestor);

	virtual math::Vec getRelativeOffset(math::Vec v, Widget *ancestor);

	math::Vec getAbsoluteOffset(math::Vec v) {
		return getRelativeOffset(v, nullptr);
	}

	virtual float getRelativeZoom(Widget *ancestor);

	float getAbsoluteZoom() {
		return getRelativeZoom(nullptr);
	}
	/** Returns a subset of the given Rect bounded by the box of this widget and all ancestors.
	*/
	virtual math::Rect getViewport(math::Rect r = math::Rect::inf());

	template<class T>
	T *getAncestorOfType() {
		if (!parent)
			return nullptr;
		T *p = dynamic_cast<T *>(parent);
		if (p)
			return p;
		return parent->getAncestorOfType<T>();
	}

	template<class T>
	T *getFirstDescendantOfType() {
		for (Widget *child : children) {
			T *c = dynamic_cast<T *>(child);
			if (c)
				return c;
			c = child->getFirstDescendantOfType<T>();
			if (c)
				return c;
		}
		return nullptr;
	}

	bool hasChild(Widget *child);
	void addChild(Widget *child);
	void addChildBottom(Widget *child);
	void addChildBelow(Widget *child, Widget *sibling);
	void addChildAbove(Widget *child, Widget *sibling);
	void removeChild(Widget *child);
	void clearChildren();
	virtual void step();

	struct DrawArgs {
		NVGcontext *vg = nullptr;
		/** Local box representing the visible viewport. */
		math::Rect clipBox;
		NVGLUframebuffer *fb = nullptr;
	};

	virtual void draw(const DrawArgs &args);
	//DEPRECATED virtual void draw(NVGcontext* vg) {}

	/** Draw additional layers.

	Custom widgets may draw its children multiple times on different layers, passing an arbitrary layer number each time.
	Layer 0 calls children's draw().
	When overriding, always wrap draw commands in `if (layer == ...) {}` to avoid drawing on all layers.
	When overriding, call the superclass's `drawLayer(args, layer)` to recurse to children.
	*/
	virtual void drawLayer(const DrawArgs &args, int layer);

	void drawChild(Widget *child, const DrawArgs &args, int layer = 0);

	// Events

	/** Recurses an event to all visible Widgets */
	template<typename TMethod, class TEvent>
	void recurseEvent(TMethod f, const TEvent &e) {
		//not supported
	}

	/** Recurses an event to all visible Widgets until it is consumed. */
	template<typename TMethod, class TEvent>
	void recursePositionEvent(TMethod f, const TEvent &e) {
		//not supported
	}

	using BaseEvent = widget::BaseEvent;

	/** An event prototype with a vector position. */
	struct PositionBaseEvent {
		/** The pixel coordinate where the event occurred, relative to the Widget it is called on. */
		math::Vec pos;
	};

	/** Occurs every frame when the mouse is hovering over a Widget.
	Recurses.
	Consume this event to allow Enter and Leave to occur.
	*/
	struct HoverEvent : BaseEvent, PositionBaseEvent {
		/** Change in mouse position since the last frame. Can be zero. */
		math::Vec mouseDelta;
	};
	virtual void onHover(const HoverEvent &e) {
		recursePositionEvent(&Widget::onHover, e);
	}

	struct ButtonEvent : BaseEvent, PositionBaseEvent {
		int button{};
		int action{};
		int mods{};
	};
	virtual void onButton(const ButtonEvent &e) {
		recursePositionEvent(&Widget::onButton, e);
	}

	struct DoubleClickEvent : BaseEvent {};
	virtual void onDoubleClick(const DoubleClickEvent &e) {
	}

	/** An event prototype with a GLFW key. */
	struct KeyBaseEvent {
		int key;
		int scancode;
		std::string keyName;
		int action;
		int mods;
	};

	struct HoverKeyEvent : BaseEvent, PositionBaseEvent, KeyBaseEvent {};
	virtual void onHoverKey(const HoverKeyEvent &e) {
		recursePositionEvent(&Widget::onHoverKey, e);
	}

	struct TextBaseEvent {
		int codepoint{};
	};
	struct HoverTextEvent : BaseEvent, PositionBaseEvent, TextBaseEvent {};
	virtual void onHoverText(const HoverTextEvent &e) {
		recursePositionEvent(&Widget::onHoverText, e);
	}

	struct HoverScrollEvent : BaseEvent, PositionBaseEvent {
		math::Vec scrollDelta;
	};
	virtual void onHoverScroll(const HoverScrollEvent &e) {
		recursePositionEvent(&Widget::onHoverScroll, e);
	}

	/** Occurs when a Widget begins consuming the Hover event.
	Must consume the Hover event to receive this event.
	The target sets `hoveredWidget`, which allows Leave to occur.
	*/
	struct EnterEvent : BaseEvent {};
	virtual void onEnter(const EnterEvent &e) {
	}

	/** Occurs when a different Widget is entered.
	Must consume the Hover event (when a Widget is entered) to receive this event.
	*/
	struct LeaveEvent : BaseEvent {};
	virtual void onLeave(const LeaveEvent &e) {
	}

	/** Occurs when a Widget begins consuming the Button press event for the left mouse button.
	Must consume the Button event (on left button press) to receive this event.
	The target sets `selectedWidget`, which allows SelectText and SelectKey to occur.
	*/
	struct SelectEvent : BaseEvent {};
	virtual void onSelect(const SelectEvent &e) {
	}

	/** Occurs when a different Widget is selected.
	Must consume the Button event (on left button press, when the Widget is selected) to receive this event.
	*/
	struct DeselectEvent : BaseEvent {};
	virtual void onDeselect(const DeselectEvent &e) {
	}

	/** Occurs when a key is pressed, released, or repeated while a Widget is selected.
	Must consume to prevent HoverKey from being triggered.
	*/
	struct SelectKeyEvent : BaseEvent, KeyBaseEvent {};
	virtual void onSelectKey(const SelectKeyEvent &e) {
	}

	/** Occurs when text is typed while a Widget is selected.
	Must consume to prevent HoverKey from being triggered.
	*/
	struct SelectTextEvent : BaseEvent, TextBaseEvent {};
	virtual void onSelectText(const SelectTextEvent &e) {
	}

	struct DragBaseEvent : BaseEvent {
		int button{};
	};
	/** Occurs when a Widget begins being dragged.
	Must consume the Button event (on press) to receive this event.
	The target sets `draggedWidget`, which allows DragEnd, DragMove, DragHover, DragEnter, and DragDrop to occur.
	*/
	struct DragStartEvent : DragBaseEvent {};
	virtual void onDragStart(const DragStartEvent &e) {
	}

	/** Occurs when a Widget stops being dragged by releasing the mouse button.
	Must consume the Button event (on press, when the Widget drag begins) to receive this event.
	*/
	struct DragEndEvent : DragBaseEvent {};
	virtual void onDragEnd(const DragEndEvent &e) {
	}

	/** Occurs every frame on the dragged Widget.
	Must consume the Button event (on press, when the Widget drag begins) to receive this event.
	*/
	struct DragMoveEvent : DragBaseEvent {
		/** Change in mouse position since the last frame. Can be zero. */
		math::Vec mouseDelta;
	};
	virtual void onDragMove(const DragMoveEvent &e) {
	}

	/** Occurs every frame when the mouse is hovering over a Widget while another Widget (possibly the same one) is being dragged.
	Recurses.
	Consume this event to allow DragEnter and DragLeave to occur.
	*/
	struct DragHoverEvent : DragBaseEvent, PositionBaseEvent {
		/** The dragged widget */
		Widget *origin = nullptr;
		/** Change in mouse position since the last frame. Can be zero. */
		math::Vec mouseDelta;
	};
	virtual void onDragHover(const DragHoverEvent &e) {
		recursePositionEvent(&Widget::onDragHover, e);
	}

	/** Occurs when the mouse enters a Widget while dragging.
	Must consume the DragHover event to receive this event.
	The target sets `draggedWidget`, which allows DragLeave to occur.
	*/
	struct DragEnterEvent : DragBaseEvent {
		/** The dragged widget */
		Widget *origin = nullptr;
	};
	virtual void onDragEnter(const DragEnterEvent &e) {
	}

	/** Occurs when the mouse leaves a Widget while dragging.
	Must consume the DragHover event (when the Widget is entered) to receive this event.
	*/
	struct DragLeaveEvent : DragBaseEvent {
		/** The dragged widget */
		Widget *origin = nullptr;
	};
	virtual void onDragLeave(const DragLeaveEvent &e) {
	}

	/** Occurs when the mouse button is released over a Widget while dragging.
	Must consume the Button event (on release) to receive this event.
	*/
	struct DragDropEvent : DragBaseEvent {
		/** The dragged widget */
		Widget *origin = nullptr;
	};
	virtual void onDragDrop(const DragDropEvent &e) {
	}

	/** Occurs when a selection of files from the operating system is dropped onto a Widget.
	Recurses.
	*/
	struct PathDropEvent : BaseEvent, PositionBaseEvent {
		PathDropEvent(const std::vector<std::string> &paths)
			: paths(paths) {
		}

		/** List of file paths in the dropped selection */
		const std::vector<std::string> &paths;
	};
	virtual void onPathDrop(const PathDropEvent &e) {
		recursePositionEvent(&Widget::onPathDrop, e);
	}

	/** Occurs after a certain action is triggered on a Widget.
	The concept of an "action" is defined by the type of Widget.
	*/
	struct ActionEvent : BaseEvent {};
	virtual void onAction(const ActionEvent &e) {
	}

	/** Occurs after the value of a Widget changes.
	The concept of a "value" is defined by the type of Widget.
	*/
	struct ChangeEvent : BaseEvent {};
	virtual void onChange(const ChangeEvent &e) {
	}

	/** Occurs when the pixel buffer of this module must be refreshed.
	Recurses.
	*/
	struct DirtyEvent : BaseEvent {};
	virtual void onDirty(const DirtyEvent &e) {
		recurseEvent(&Widget::onDirty, e);
	}

	/** Occurs after a Widget's position is set by Widget::setPosition().
	*/
	struct RepositionEvent : BaseEvent {};
	virtual void onReposition(const RepositionEvent &e) {
	}

	/** Occurs after a Widget's size is set by Widget::setSize().
	*/
	struct ResizeEvent : BaseEvent {};
	virtual void onResize(const ResizeEvent &e) {
	}

	/** Occurs after a Widget is added to a parent.
	*/
	struct AddEvent : BaseEvent {};
	virtual void onAdd(const AddEvent &e) {
	}

	/** Occurs before a Widget is removed from its parent.
	*/
	struct RemoveEvent : BaseEvent {};
	virtual void onRemove(const RemoveEvent &e) {
	}

	/** Occurs after a Widget is shown with Widget::show().
	Recurses.
	*/
	struct ShowEvent : BaseEvent {};
	virtual void onShow(const ShowEvent &e) {
		recurseEvent(&Widget::onShow, e);
	}

	/** Occurs after a Widget is hidden with Widget::hide().
	Recurses.
	*/
	struct HideEvent : BaseEvent {};
	virtual void onHide(const HideEvent &e) {
		recurseEvent(&Widget::onHide, e);
	}

	/** Occurs after the Window (including OpenGL and NanoVG contexts) are created.
	Recurses.
	*/
	struct ContextCreateEvent : BaseEvent {
		NVGcontext *vg{};
	};
	virtual void onContextCreate(const ContextCreateEvent &e) {
		recurseEvent(&Widget::onContextCreate, e);
	}

	/** Occurs before the Window (including OpenGL and NanoVG contexts) are destroyed.
	Recurses.
	*/
	struct ContextDestroyEvent : BaseEvent {
		NVGcontext *vg{};
	};
	virtual void onContextDestroy(const ContextDestroyEvent &e) {
		recurseEvent(&Widget::onContextDestroy, e);
	}
};

} // namespace rack::widget

/** Deprecated Rack v1 event namespace.
Use events defined in the widget::Widget class instead of this `event::` namespace in new code.
*/
namespace event
{
using Base = rack::widget::BaseEvent;
using PositionBase = rack::widget::Widget::PositionBaseEvent;
using KeyBase = rack::widget::Widget::KeyBaseEvent;
using TextBase = rack::widget::Widget::TextBaseEvent;
using Hover = rack::widget::Widget::HoverEvent;
using Button = rack::widget::Widget::ButtonEvent;
using DoubleClick = rack::widget::Widget::DoubleClickEvent;
using HoverKey = rack::widget::Widget::HoverKeyEvent;
using HoverText = rack::widget::Widget::HoverTextEvent;
using HoverScroll = rack::widget::Widget::HoverScrollEvent;
using Enter = rack::widget::Widget::EnterEvent;
using Leave = rack::widget::Widget::LeaveEvent;
using Select = rack::widget::Widget::SelectEvent;
using Deselect = rack::widget::Widget::DeselectEvent;
using SelectKey = rack::widget::Widget::SelectKeyEvent;
using SelectText = rack::widget::Widget::SelectTextEvent;
using DragBase = rack::widget::Widget::DragBaseEvent;
using DragStart = rack::widget::Widget::DragStartEvent;
using DragEnd = rack::widget::Widget::DragEndEvent;
using DragMove = rack::widget::Widget::DragMoveEvent;
using DragHover = rack::widget::Widget::DragHoverEvent;
using DragEnter = rack::widget::Widget::DragEnterEvent;
using DragLeave = rack::widget::Widget::DragLeaveEvent;
using DragDrop = rack::widget::Widget::DragDropEvent;
using PathDrop = rack::widget::Widget::PathDropEvent;
using Action = rack::widget::Widget::ActionEvent;
using Change = rack::widget::Widget::ChangeEvent;
using Dirty = rack::widget::Widget::DirtyEvent;
using Reposition = rack::widget::Widget::RepositionEvent;
using Resize = rack::widget::Widget::ResizeEvent;
using Add = rack::widget::Widget::AddEvent;
using Remove = rack::widget::Widget::RemoveEvent;
using Show = rack::widget::Widget::ShowEvent;
using Hide = rack::widget::Widget::HideEvent;

} // namespace event
