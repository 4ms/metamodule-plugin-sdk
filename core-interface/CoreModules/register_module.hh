#pragma once
#include "CoreProcessor.hh"
#include "elements/element_info_view.hh"
#include <functional>
#include <memory>

namespace MetaModule
{

// This file contains several overloads for the `register_module` function.
// Each one does the same thing, but allows you more or less customization.
//
// The `register_module` function is used to register your module class with the MetaModule.
// Typically you will call it for each module in the init() function:
// void init() {
//    register_module<MyModule, MyModuleInfo>();
// }
//
// However, you also can call it during static global initialization:
// struct MyModule {
//     static inline bool mymodule_ok = register_module<MyModule, MyModuleInfo>();
//
//
// If you are using the rack-interface to port a plugin from VCV Rack, then calling
// `pluginInstance->addModel(myModel)` will call register_module for you.
//
//
// The brand:
// Every module in a plugin is registered under the plugin's brand slug. That's the
// "slug" in plugin.json, or "MetaModuleBrandSlug" in plugin-mm.json if that's present.
// The SDK's create_plugin() reads it from those files and defines METAMODULE_PLUGIN_BRAND_SLUG,
// which the overloads without a brand parameter use.
//
// The overloads that take a brand are deprecated for plugins: a brand that doesn't match
// the plugin's slug breaks finding the plugin's modules in patches, the plugin's
// metadata (plugin.json and plugin-mm.json), and the module/plugin lists on the
// website. The MetaModule prints a warning when a plugin registers a module under any
// other brand.
//

#if defined(METAMODULE_PLUGIN_BRAND_SLUG)
#define METAMODULE_DEPRECATED_BRAND_ARG                                                                                \
	[[deprecated("Don't pass a brand: use the overload without one, which registers the module under the plugin's "   \
				 "brand slug (\"slug\" in plugin.json, or \"MetaModuleBrandSlug\" in plugin-mm.json)")]]
#else
#define METAMODULE_DEPRECATED_BRAND_ARG
#endif

// This an internally used type (factory function):
//
using CreateModuleFunc = std::function<std::unique_ptr<CoreProcessor>()>;

//
// Register a module.
// Example:
// bool ok = register_module("ThisModule", creationFunction, thisModuleInfoView, "mybrand/faceplate.png");
//
// Normally you won't call this form of register_module: it's for special-case situations where you need to handle some complex
// behavior on module creation. Also, since it takes a ModuleInfoView, it's probably also only useful if you
// create and populate the info class manually (like NativeExample:SimpleGain does)
//
// Deprecated for plugins: use the overload without a brand_slug, below
METAMODULE_DEPRECATED_BRAND_ARG
bool register_module(std::string_view brand_slug,
					 std::string_view module_slug,
					 CreateModuleFunc funcCreate,
					 ModuleInfoView const &info,
					 std::string_view faceplate_filename);

// The overloads below call the deprecated ones internally: only calls from plugin code should warn
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// Register a module using the brand slug, module slug, and faceplate name.
// Pass the module class and info classes as template parameters.
// The info class must have a `slug` member and a `png_filename` member.
//
// Deprecated for plugins: use register_module<Module, ModuleInfo>(), below
template<typename ModuleT, typename ModuleInfoT>
METAMODULE_DEPRECATED_BRAND_ARG bool register_module(std::string_view brand_name) {
	return register_module(
		brand_name,
		ModuleInfoT::slug,
		[]() { return std::make_unique<ModuleT>(); },
		ModuleInfoView::makeView<ModuleInfoT>(),
		ModuleInfoT::png_filename);
}

// Register a module using the brand slug, module slug, and faceplate name.
// Pass the module class and info classes as template parameters.
//
// Deprecated for plugins: use register_module<Module, ModuleInfo>(module_slug, faceplate_filename), below
template<typename ModuleT, typename ModuleInfoT>
METAMODULE_DEPRECATED_BRAND_ARG bool
register_module(std::string_view brand_name, std::string_view module_slug, std::string_view faceplate_filename) {
	return register_module(
		brand_name,
		module_slug,
		[]() { return std::make_unique<ModuleT>(); },
		ModuleInfoView::makeView<ModuleInfoT>(),
		faceplate_filename);
}

// Register a module with a info class you built manually.
// The default creation function will be used.
//
// Deprecated for plugins: use register_module<ThisModule>(module_slug, info, faceplate_filename), below
template<typename ModuleT>
METAMODULE_DEPRECATED_BRAND_ARG bool register_module(std::string_view brand_name,
													 std::string_view module_slug,
													 ModuleInfoView const &info,
													 std::string_view faceplate_filename) {
	return register_module(
		brand_name, module_slug, []() { return std::make_unique<ModuleT>(); }, info, faceplate_filename);
}

#if defined(METAMODULE_PLUGIN_BRAND_SLUG)

//
// The overloads for plugins: these register the module under the plugin's brand slug
//

// Register a module.
// Pass the module class and info classes as template parameters.
// The info class must have a `slug` member and a `png_filename` member.
// Example:
// bool ok = register_module<Module, ModuleInfo>();
//
template<typename ModuleT, typename ModuleInfoT>
bool register_module() {
	return register_module<ModuleT, ModuleInfoT>(METAMODULE_PLUGIN_BRAND_SLUG);
}

// Register a module using the module slug and faceplate name.
// Pass the module class and info classes as template parameters.
// Example:
// bool ok = register_module<Module, ModuleInfo>("ThisModule", "mybrand/faceplate.png");
//
template<typename ModuleT, typename ModuleInfoT>
bool register_module(std::string_view module_slug, std::string_view faceplate_filename) {
	return register_module<ModuleT, ModuleInfoT>(METAMODULE_PLUGIN_BRAND_SLUG, module_slug, faceplate_filename);
}

// Register a module with a info class you built manually.
// The default creation function will be used.
// Example:
// bool ok = register_module<ThisModule>("ThisModule", thisModuleInfoView, "mybrand/faceplate.png");
//
template<typename ModuleT>
bool register_module(std::string_view module_slug, ModuleInfoView const &info, std::string_view faceplate_filename) {
	return register_module<ModuleT>(METAMODULE_PLUGIN_BRAND_SLUG, module_slug, info, faceplate_filename);
}

// Register a module with a custom creation function.
// Example:
// bool ok = register_module("ThisModule", creationFunction, thisModuleInfoView, "mybrand/faceplate.png");
//
inline bool register_module(std::string_view module_slug,
							CreateModuleFunc funcCreate,
							ModuleInfoView const &info,
							std::string_view faceplate_filename) {
	return register_module(METAMODULE_PLUGIN_BRAND_SLUG, module_slug, std::move(funcCreate), info, faceplate_filename);
}

#endif

#pragma GCC diagnostic pop

} // namespace MetaModule
