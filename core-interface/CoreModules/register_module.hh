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
//    register_module<MyModule, MyModuleInfo>("MyBrand");
// }
//
// However, you also can call it during static global initialization:
// struct MyModule {
//     static inline bool mymodule_ok = register_module<MyModule, MyModuleInfo>("MyBrand");
//
//
// If you are using the rack-interface to port a plugin from VCV Rack, then calling
// `pluginInstance->addModel(myModel)` will call register_module for you.
//

// This an internally used type (factory function):
//
using CreateModuleFunc = std::function<std::unique_ptr<CoreProcessor>()>;

//
// Register a module.
// Example:
// bool ok = register_module("MyBrand", "ThisModule", creationFunction, thisModuleInfoView, "mybrand/faceplate.png");
//
// Normally you won't call this form of register_module: it's for special-case situations where you need to handle some complex
// behavior on module creation. Also, since it takes a ModuleInfoView, it's probably also only useful if you
// create and populate the info class manually (like NativeExample:SimpleGain does)
//
bool register_module(std::string_view brand_slug,
					 std::string_view module_slug,
					 CreateModuleFunc funcCreate,
					 ModuleInfoView const &info,
					 std::string_view faceplate_filename);

// Register a module using the brand slug, module slug, and faceplate name.
// Pass the module class and info classes as template parameters.
// The info class must have a `slug` member and a `png_filename` member.
// Example:
// book ok = register_module<Module, ModuleInfo>("MyBrand");
//
template<typename ModuleT, typename ModuleInfoT>
bool register_module(std::string_view brand_name) {
	return register_module(
		brand_name,
		ModuleInfoT::slug,
		[]() { return std::make_unique<ModuleT>(); },
		ModuleInfoView::makeView<ModuleInfoT>(),
		ModuleInfoT::png_filename);
}

// Register a module using the brand slug, module slug, and faceplate name.
// Pass the module class and info classes as template parameters.
// Example:
// book ok = register_module<Module, ModuleInfo>("MyBrand", "ThisModule", "mybrand/faceplate.png");
//
template<typename ModuleT, typename ModuleInfoT>
bool register_module(std::string_view brand_name, std::string_view module_slug, std::string_view faceplate_filename) {
	return register_module(
		brand_name,
		module_slug,
		[]() { return std::make_unique<ModuleT>(); },
		ModuleInfoView::makeView<ModuleInfoT>(),
		faceplate_filename);
}

// Register a module with a info class you built manually.
// The default creation function will be used.
// Example:
// bool ok = register_module<ThisModule>("MyBrand", "ThisModule", thisModuleInfoView, "mybrand/faceplate.png");
//
template<typename ModuleT>
bool register_module(std::string_view brand_name,
					 std::string_view module_slug,
					 ModuleInfoView const &info,
					 std::string_view faceplate_filename) {
	return register_module(
		brand_name, module_slug, []() { return std::make_unique<ModuleT>(); }, info, faceplate_filename);
}

} // namespace MetaModule

