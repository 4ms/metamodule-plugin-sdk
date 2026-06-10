#pragma once
#include "CoreModules/CoreProcessor.hh"
#include <app/ModuleWidget.hpp>
#include <memory>
#include <plugin/Model.hpp>
#include <string_view>

namespace rack
{

// Holds the Model registered for each Module/ModuleWidget type, so that
// create_vcv_module (a captureless function pointer) can set module->model
template<typename ModuleT, typename ModuleWidgetT>
struct RegisteredModel {
	static inline plugin::Model *model = nullptr;
};

template<typename ModuleT, typename ModuleWidgetT>
std::unique_ptr<CoreProcessor> create_vcv_module() {
	auto module = std::make_unique<ModuleT>();
	module->model = RegisteredModel<ModuleT, ModuleWidgetT>::model;
	module->module_widget = std::make_shared<ModuleWidgetT>(module.get());
	return module;
}

// Allocates and constructs a Model that can
// create the given Module and ModuleWidget subclasses
// To match VCV Rack API, ownership of the Model is passed to the caller.
template<class ModuleT, class ModuleWidgetT>
requires(std::derived_from<ModuleWidgetT, app::ModuleWidget>) && (std::derived_from<ModuleT, rack::engine::Module>)

plugin::Model *createModel(std::string_view slug) {
	struct ModelT : plugin::Model {

		rack::engine::Module *createModule() override {
			engine::Module *m = new ModuleT;
			m->model = this;
			return m;
		}

		app::ModuleWidget *createModuleWidget(engine::Module *m) override {
			auto tm = static_cast<ModuleT *>(m);
			app::ModuleWidget *mw = new ModuleWidgetT(tm);
			mw->setModel(this);
			return mw;
		}
	};

	plugin::Model *model = new ModelT;
	model->slug = slug;
	RegisteredModel<ModuleT, ModuleWidgetT>::model = model;
	model->creation_func = create_vcv_module<ModuleT, ModuleWidgetT>;
	return model;
}

} // namespace rack

// using namespace rack;
