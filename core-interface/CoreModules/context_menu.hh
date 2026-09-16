#pragma once
#include "CoreProcessor.hh"
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace MetaModule
{

// Context menus for native (CoreProcessor) modules.
// See docs/context-menus.md

// A single row in the module's context menu
struct ContextMenuItem {
	enum class Type {
		Action,	  // Clicking calls on_change(module, index, 1.f)
		Checkbox, // Shows a checkmark when `checked` is true. Clicking calls on_change(module, index, new_state ? 1.f : 0.f)
		Slider,	  // Continuous 0..1 value, edited in a popup. Calls on_change(module, index, value)
		Label,	  // Non-interactive text (e.g. a heading)
		Divider,  // Horizontal separator line
	};

	Type type = Type::Action;
	std::string name;
	bool checked = false;	// Checkbox only
	float value = 0.f;		// Slider only
	std::string value_text; // Optional text shown after the name (e.g. a Slider's current value)
};

struct ContextMenuHandlers {
	// Returns the rows to show. Called each time the menu is opened or refreshed
	// (e.g. after every click), so it should reflect the module's current state.
	std::function<std::vector<ContextMenuItem>(CoreProcessor *module)> get_items;

	// Called when the user clicks an Action or Checkbox row, or adjusts a Slider row.
	// `index` is the row's position in the vector returned by get_items().
	// `value` is 1 for Actions, 0/1 for Checkboxes, and the slider value 0..1 for Sliders
	std::function<void(CoreProcessor *module, unsigned index, float value)> on_change;
};

// Register a context menu for a module type, returns false if the module isn't registered.
// Typically only used for fine-tuned control, prefer using register_context_menu<ModuleT, ModuleInfoT>()
bool register_context_menu(std::string_view brand_slug, std::string_view module_slug, ContextMenuHandlers handlers);

// Register a context menu using member functions of your module class.
//
// Example:
// register_context_menu<MyModule, MyModuleInfo>("MyBrand", &MyModule::menu_items, &MyModule::menu_changed);
template<typename ModuleT, typename ModuleInfoT>
bool register_context_menu(std::string_view brand_slug,
						   std::vector<ContextMenuItem> (ModuleT::*get_items)(),
						   void (ModuleT::*on_change)(unsigned index, float value)) {
	return register_context_menu(
		brand_slug,
		ModuleInfoT::slug,
		{
			.get_items = [get_items](CoreProcessor *module) { return (static_cast<ModuleT *>(module)->*get_items)(); },
			.on_change = [on_change](CoreProcessor *module,
									 unsigned index,
									 float value) { (static_cast<ModuleT *>(module)->*on_change)(index, value); },
		});
}

} // namespace MetaModule
