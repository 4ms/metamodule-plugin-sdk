#pragma once
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/element_counter.hh"
#include <common.hpp>
#include <deque>
#include <jansson.h>
#include <list>
#include <memory>
#include <plugin/Plugin.hpp>
#include <vector>

namespace rack::ui
{
struct Menu;
} // namespace rack::ui

namespace rack::app
{
struct ModuleWidget;
} // namespace rack::app

namespace rack::engine
{
struct Module;
} // namespace rack::engine

namespace rack::plugin
{

struct Model {
	Plugin *plugin = nullptr;

	//MetaModule:
	using CreateModuleFunc = std::unique_ptr<CoreProcessor> (*)();
	CreateModuleFunc creation_func;

	std::string slug{};

	std::string name;
	std::list<int> tagIds;
	std::string description;
	std::string manualUrl;
	std::string modularGridUrl;

	bool hidden = false;

	std::vector<MetaModule::Element> elements;
	std::vector<ElementCount::Indices> indices;
	std::deque<std::string> strings;

	virtual ~Model() = default;

	virtual engine::Module *createModule() {
		return nullptr;
	}

	virtual app::ModuleWidget *createModuleWidget(engine::Module *m) {
		return nullptr;
	}

	void move_strings();
	std::string_view add_string(std::string_view str);
	void debug_dump_strings();

	void fromJson(json_t *rootJ);

	std::string getFullName();
	std::string getFactoryPresetDirectory();
	std::string getUserPresetDirectory();
	std::string getManualUrl();
	void appendContextMenu(ui::Menu *menu, bool inBrowser = false);
	bool isFavorite();
	void setFavorite(bool favorite);
};

} // namespace rack::plugin

