#pragma once
#include <app/Knob.hpp>
#include <app/ModuleLightWidget.hpp>
#include <app/ParamWidget.hpp>
#include <app/PortWidget.hpp>
#include <app/SvgButton.hpp>
#include <app/SvgKnob.hpp>
#include <app/SvgPanel.hpp>
#include <app/SvgSlider.hpp>
#include <app/common.hpp>
#include <componentlibrary.hpp>
#include <history.hpp>
#include <metamodule/VCVTextDisplay.hpp>
#include <plugin/Model.hpp>
#include <ui/Menu.hpp>
#include <widget/OpaqueWidget.hpp>

namespace rack::engine
{
struct Module;
};

namespace rack::app
{

struct ModuleWidget : widget::Widget {
	struct Internal;
	Internal *internal;

	plugin::Model *model = nullptr;

	engine::Module *module = nullptr;

	ModuleWidget();
	~ModuleWidget() override;

	plugin::Model *getModel();
	void setModule(engine::Module *m);

	engine::Module *getModule();

	template<class TModule>
	TModule *getModule() {
		return dynamic_cast<TModule *>(getModule());
	}

	void setModel(plugin::Model *m);

	app::SvgPanel *getPanel();

	// In Rack, this is setPanel(Widget*)
	// but we use setPanel(SvgPanel*) in order to call addChildBottom(SvgPanel*)
	// because we need to know which widget is the SvgPanel so we make sure to use it as the faceplate
	void setPanel(app::SvgPanel *newpanel);
	void setPanel(std::shared_ptr<window::Svg> svg);

	// Catch-all:
	void addChild(Widget *w);

	// Rendered as images:
	void addChild(MetaModule::VCVTextDisplay *widget);
	void addChild(widget::SvgWidget *w);
	void addChild(app::SvgButton *w);
	void addChild(app::SvgScrew *widget);

	// Panels:
	void addChild(SvgPanel *child);
	void addChildBottom(SvgPanel *child);

	// Params:
	void addParam(ParamWidget *param);
	void addParam(Knob *param);
	void addParam(rack::componentlibrary::Rogan *widget);
	void addParam(app::SvgKnob *param);
	void addParam(app::SvgSlider *param);
	void addParam(app::SvgSwitch *param);
	void addParam(app::SliderKnob *param);

	// Button + Light:
	template<typename TLightBase>
	void addParam(componentlibrary::VCVLightBezel<TLightBase> *widget) {
		addLightSwitch(widget, widget->getLight());
	}
	void addLightSwitch(app::SvgSwitch *widget, app::ModuleLightWidget *light);

	// Slider + Light:
	template<typename TBase, typename TLightBase>
	void addParam(componentlibrary::LightSlider<TBase, TLightBase> *widget) {
		addLightSlider(widget, widget->getLight());
	}
	void addLightSlider(app::SvgSlider *slider, app::ModuleLightWidget *light);

	// Lights:
	void addChild(app::ModuleLightWidget *lightWidget);

	template<typename LightBaseT>
	void addChild(componentlibrary::TSvgLight<LightBaseT> *widget) {
		if (widget->sw && widget->sw->svg)
			addSvgLight(widget->sw->svg->filename(), widget);
		else
			addChild(widget);
	}
	void addSvgLight(std::string_view image, app::ModuleLightWidget *widget);

	// Ports:
	void addInput(PortWidget *input);
	void addInput(SvgPort *input);
	void addOutput(PortWidget *output);
	void addOutput(SvgPort *output);

	ParamWidget *getParam(int paramId);
	PortWidget *getInput(int portId);
	PortWidget *getOutput(int portId);
	std::vector<ParamWidget *> getParams();
	std::vector<PortWidget *> getPorts();
	std::vector<PortWidget *> getInputs();
	std::vector<PortWidget *> getOutputs();

	struct WidgetElement {
		unsigned element_idx;
		rack::widget::Widget *widget;
	};
	__attribute__((visibility("hidden"))) std::vector<WidgetElement> &get_drawable_widgets();
	__attribute__((visibility("hidden"))) void populate_elements_indices(rack::plugin::Model *model);

	///////////////////////

	void draw(const DrawArgs &args) override;
	void drawLayer(const DrawArgs &args, int layer) override;
	virtual void appendContextMenu(ui::Menu *menu) {
	}
	void onHover(const HoverEvent &e) override;
	void onHoverKey(const HoverKeyEvent &e) override;
	void onButton(const ButtonEvent &e) override;
	void onDragStart(const DragStartEvent &e) override;
	void onDragEnd(const DragEndEvent &e) override;
	void onDragMove(const DragMoveEvent &e) override;
	void onDragHover(const DragHoverEvent &e) override;
	json_t *toJson();
	void fromJson(json_t *rootJ);
	bool pasteJsonAction(json_t *rootJ);
	void copyClipboard();
	bool pasteClipboardAction();
	void load(std::string filename);
	void loadAction(std::string filename);
	void loadTemplate();
	void loadDialog();
	void save(std::string filename);
	void saveTemplate();
	void saveTemplateDialog();
	bool hasTemplate();
	void clearTemplate();
	void clearTemplateDialog();
	void saveDialog();
	void disconnect();
	void resetAction();
	void randomizeAction();
	void appendDisconnectActions(history::ComplexAction *complexAction);
	void disconnectAction();
	void cloneAction(bool cloneCables = true);
	void bypassAction(bool bypassed);
	void removeAction();
	void createContextMenu();
	math::Vec getGridPosition();
	void setGridPosition(math::Vec pos);
	math::Vec getGridSize();
	math::Rect getGridBox();
};

} // namespace rack::app
