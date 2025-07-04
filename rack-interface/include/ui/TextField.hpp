#pragma once
#include <context.hpp>
#include <ui/common.hpp>
#include <widget/OpaqueWidget.hpp>

namespace rack::ui
{

struct TextField : widget::OpaqueWidget {
	std::string text;
	std::string placeholder;

	bool password = false;
	bool multiline = false;

	int cursor = 0;
	int selection = 0;

	Widget *prevField = nullptr;
	Widget *nextField = nullptr;

	TextField();
	void draw(const DrawArgs &args) override;
	void onDragHover(const DragHoverEvent &e) override;
	void onButton(const ButtonEvent &e) override;
	void onSelectText(const SelectTextEvent &e) override;
	void onSelectKey(const SelectKeyEvent &e) override;
	virtual int getTextPosition(math::Vec mousePos);

	std::string getText();
	void setText(std::string text);
	void selectAll();
	std::string getSelectedText();
	void insertText(std::string text);
	void copyClipboard();
	void cutClipboard();
	void pasteClipboard();
	void cursorToPrevWord();
	void cursorToNextWord();
	void createContextMenu();
};

struct PasswordField : TextField {
	PasswordField() {
		password = true;
	}
};

} // namespace rack::ui
