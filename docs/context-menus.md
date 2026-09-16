# Context menus (native modules)

Native (`CoreProcessor`) modules can provide a context menu, shown under an
"Options" entry at the bottom of the module's parameter list in the GUI. This
is the same place a VCV-ported module's `appendContextMenu()` items appear.

The menu is registered once per module type (slug), usually in your plugin's
`init()` right after `register_module()`. When the user opens the menu, the
host calls your callbacks with the module instance whose menu is open, so the
menu can show and change that instance's state.

The API is in `CoreModules/context_menu.hh`.

## Menu items

Each row of the menu is a `ContextMenuItem`, which has a `type`:

- `Action` — a clickable row. Clicking it calls `on_change(module, index, 1.f)`.
- `Checkbox` — shows a checkmark when its `checked` field is true. Clicking it
  calls `on_change(module, index, value)`, with `value` = 1 if the box should
  now be checked, or 0 if it should now be unchecked.
- `Slider` — a continuous value in the range 0..1 (its `value` field).
  Clicking it opens a value editor; while the user adjusts it,
  `on_change(module, index, value)` is called.
- `Label` — non-interactive text, e.g. a heading.
- `Divider` — a horizontal separator line.

Any row can set `value_text`, which is shown after the name (e.g. "50%" for a
Slider, or the currently selected option for an Action that cycles through
choices).

`index` is the row's position in the vector returned by `get_items()`.

## Registering with member functions

The easiest way is to add two member functions to your module class and
register them with `register_context_menu<ModuleT, ModuleInfoT>()`:

```c++
#include "CoreModules/context_menu.hh"
#include "CoreModules/register_module.hh"

class MyModule : public CoreProcessor {
public:
    std::vector<ContextMenuItem> menu_items() {
        using Type = ContextMenuItem::Type;
        return {
            {.type = Type::Label, .name = "Options"},
            {.type = Type::Checkbox, .name = "Invert", .checked = inverted},
            {.type = Type::Slider, .name = "Depth", .value = depth, .value_text = std::to_string(int(depth * 100)) + "%"},
        };
    }

    void menu_changed(unsigned index, float value) {
        if (index == 1)
            inverted = value > 0.5f;
        else if (index == 2)
            depth = value;
    }

    // ... update(), etc

private:
    bool inverted = false;
    float depth = 0.5f;
};

void init() {
    register_module<MyModule, MyModuleInfo>("MyBrand");
    register_context_menu<MyModule, MyModuleInfo>("MyBrand", &MyModule::menu_items, &MyModule::menu_changed);
}
```

## Registering with ContextMenuHandlers

For more control, fill out a `ContextMenuHandlers` struct and pass it with the
brand and module slugs. Each callback gets a `CoreProcessor *`, which you can
`static_cast` to your module class (the host only passes modules that were
created for this slug):

```c++
register_context_menu("MyBrand", "MyModule", {
    .get_items = [](CoreProcessor *module) {
        auto m = static_cast<MyModule *>(module);
        return std::vector<ContextMenuItem>{
            {.type = ContextMenuItem::Type::Checkbox, .name = "Mute", .checked = m->mute},
        };
    },
    .on_change = [](CoreProcessor *module, unsigned index, float value) {
        static_cast<MyModule *>(module)->mute = value > 0.5f;
    },
});
```

## Notes

- The module must be registered with `register_module()` before its context
  menu is registered. `register_context_menu()` returns false otherwise.
- `get_items()` is called each time the menu is opened and after every click,
  so it should always return the module's current state.
- All callbacks are called in the GUI context, so it's safe to allocate
  memory. However, `update()` may be running at the same time on another core,
  so keep state that's shared with `update()` simple (bools, floats, or
  `std::atomic`).
- Menu-driven state is not saved automatically. Use `save_state()` and
  `load_state()` if it should persist with the patch.
- The "Options" entry only appears while the patch is playing.
- See `tests/context-menu-test` for a working example.
