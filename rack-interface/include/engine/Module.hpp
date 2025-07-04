#pragma once
#include <vector>

#include <jansson.h>

#include "metamodule/VCV_module_wrapper.hh"
#include <common.hpp>
#include <engine/Light.hpp>
#include <engine/LightInfo.hpp>
#include <engine/Param.hpp>
#include <engine/ParamQuantity.hpp>
#include <engine/Port.hpp>
#include <engine/PortInfo.hpp>
#include <plugin/Model.hpp>
#include <string.hpp>

namespace rack
{

namespace plugin
{
struct Model;
}

namespace engine
{

struct Module : VCVModuleWrapper {
	struct Internal;
	Internal *internal;

	std::shared_ptr<app::ModuleWidget> module_widget{};

	plugin::Model *model = nullptr;

	int64_t id = -1;

	std::vector<PortInfo *> inputInfos;
	std::vector<PortInfo *> outputInfos;
	std::vector<LightInfo *> lightInfos;

	struct Expander {
		int64_t moduleId = -1;
		Module *module = nullptr;
		void *producerMessage = nullptr;
		void *consumerMessage = nullptr;
		bool messageFlipRequested = false;

		void requestMessageFlip() {
			messageFlipRequested = true;
		}
	};

	Expander leftExpander;
	Expander rightExpander;

	struct BypassRoute {
		int inputId = -1;
		int outputId = -1;
	};
	std::vector<BypassRoute> bypassRoutes;

	Module();
	// DEPRECATED Module(int numParams, int numInputs, int numOutputs, int numLights = 0)
	// 	: Module() {
	// 	config(numParams, numInputs, numOutputs, numLights);
	// }
	~Module() override;

	void config(int numParams, int numInputs, int numOutputs, int numLights = 0);

	template<class TParamQuantity = ParamQuantity>
	TParamQuantity *configParam(int paramId,
								float minValue,
								float maxValue,
								float defaultValue,
								std::string name = "",
								std::string unit = "",
								float displayBase = 0.f,
								float displayMultiplier = 1.f,
								float displayOffset = 0.f) {

		if (paramId >= (int)paramQuantities.size())
			return nullptr;

		if (paramQuantities[paramId])
			delete paramQuantities[paramId];

		paramQuantities[paramId] = new TParamQuantity;
		paramQuantities[paramId]->ParamQuantity::module = this;
		paramQuantities[paramId]->ParamQuantity::paramId = paramId;
		paramQuantities[paramId]->ParamQuantity::minValue = minValue;
		paramQuantities[paramId]->ParamQuantity::maxValue = maxValue;
		paramQuantities[paramId]->ParamQuantity::defaultValue = defaultValue;
		paramQuantities[paramId]->ParamQuantity::name = name;
		paramQuantities[paramId]->ParamQuantity::unit = unit;
		paramQuantities[paramId]->ParamQuantity::displayBase = displayBase;
		paramQuantities[paramId]->ParamQuantity::displayMultiplier = displayMultiplier;
		paramQuantities[paramId]->ParamQuantity::displayOffset = displayOffset;

		Param *p = &params[paramId];
		p->value = defaultValue;

		return static_cast<TParamQuantity *>(paramQuantities[paramId]);
	}

	template<class TSwitchQuantity = SwitchQuantity>
	TSwitchQuantity *configSwitch(int paramId,
								  float minValue,
								  float maxValue,
								  float defaultValue,
								  std::string name = "",
								  std::vector<std::string> labels = {}) {
		TSwitchQuantity *sq = configParam<TSwitchQuantity>(paramId, minValue, maxValue, defaultValue, name);
		sq->snapEnabled = true;
		// TODO: does this need to be removed in MM?
		// sq->ParamQuantity::smoothEnabled = false;
		sq->labels = labels;
		return sq;
	}

	template<class TSwitchQuantity = SwitchQuantity>
	TSwitchQuantity *configButton(int paramId, std::string name = "") {
		TSwitchQuantity *sq = configParam<TSwitchQuantity>(paramId, 0.f, 1.f, 0.f, name);
		// sq->randomizeEnabled = false;
		// sq->ParamQuantity::smoothEnabled = false;
		sq->snapEnabled = true;
		return sq;
	}

	template<class TPortInfo = PortInfo>
	TPortInfo *configInput(int portId, std::string name = "") {
		if (portId >= (int)inputs.size() || portId >= (int)inputInfos.size())
			return nullptr;

		if (inputInfos[portId])
			delete inputInfos[portId];

		inputInfos[portId] = new TPortInfo;
		inputInfos[portId]->PortInfo::module = this;
		inputInfos[portId]->PortInfo::type = Port::INPUT;
		inputInfos[portId]->PortInfo::portId = portId;
		inputInfos[portId]->PortInfo::name = name;
		return inputInfos[portId];
	}

	template<class TPortInfo = PortInfo>
	TPortInfo *configOutput(int portId, std::string name = "") {
		if (portId >= (int)outputs.size() || portId >= (int)outputInfos.size())
			return nullptr;

		if (outputInfos[portId])
			delete outputInfos[portId];

		outputInfos[portId] = new TPortInfo;

		outputInfos[portId]->PortInfo::module = this;
		outputInfos[portId]->PortInfo::type = Port::OUTPUT;
		outputInfos[portId]->PortInfo::portId = portId;
		outputInfos[portId]->PortInfo::name = name;
		return outputInfos[portId];
	}

	template<class TLightInfo = LightInfo>
	TLightInfo *configLight(int lightId, std::string name = "") {
		if (lightId >= (int)lights.size() || lightId >= (int)lightInfos.size())
			return nullptr;

		if (lightInfos[lightId])
			delete lightInfos[lightId];

		lightInfos[lightId] = new TLightInfo;

		// TODO: is any of this necessary? In case a VCV module reads its own lightInfos?
		lightInfos[lightId]->LightInfo::module = this;
		lightInfos[lightId]->LightInfo::lightId = lightId;
		lightInfos[lightId]->LightInfo::name = name;

		return lightInfos[lightId];
	}

	void configBypass(int inputId, int outputId) {
		if (inputId >= (int)inputs.size())
			return;
		if (outputId >= (int)outputs.size())
			return;

		// Check that output is not yet routed
		for (BypassRoute &br : bypassRoutes) {
			if (br.outputId == outputId)
				return;
		}

		BypassRoute br;
		br.inputId = inputId;
		br.outputId = outputId;
		bypassRoutes.push_back(br);
	}

	std::string createPatchStorageDirectory();
	std::string getPatchStorageDirectory();

	plugin::Model *getModel() {
		return model;
	}
	int64_t getId() {
		return id;
	}
	int getNumParams() {
		return params.size();
	}
	Param &getParam(int index) {
		return params[index];
	}
	int getNumInputs() {
		return inputs.size();
	}
	Input &getInput(int index) {
		return inputs[index];
	}
	int getNumOutputs() {
		return outputs.size();
	}
	Output &getOutput(int index) {
		return outputs[index];
	}
	int getNumLights() {
		return lights.size();
	}
	Light &getLight(int index) {
		return lights[index];
	}
	ParamQuantity *getParamQuantity(int index) {
		if (index < 0 || index >= (int)paramQuantities.size())
			return nullptr;
		return paramQuantities[index];
	}
	PortInfo *getInputInfo(int index) {
		if (index < 0 || index >= (int)inputInfos.size())
			return nullptr;
		return inputInfos[index];
	}
	PortInfo *getOutputInfo(int index) {
		if (index < 0 || index >= (int)outputInfos.size())
			return nullptr;
		return outputInfos[index];
	}
	LightInfo *getLightInfo(int index) {
		if (index < 0 || index >= (int)lightInfos.size())
			return nullptr;
		return lightInfos[index];
	}
	Expander &getLeftExpander() {
		return leftExpander;
	}
	Expander &getRightExpander() {
		return rightExpander;
	}
	Expander &getExpander(uint8_t side) {
		return side ? rightExpander : leftExpander;
	}

	// Virtual methods

	virtual void process(const ProcessArgs &) {
		step();
	}

	virtual void step() {
	}

	virtual void processBypass(const ProcessArgs &args);

	// Called by VCVModuleWrapper
	void update(const ProcessArgs &args, bool bypassed) override;

	virtual json_t *toJson();
	virtual void fromJson(json_t *rootJ);

	virtual json_t *paramsToJson();

	virtual void paramsFromJson(json_t *rootJ);

	virtual json_t *dataToJson() {
		return nullptr;
	}
	virtual void dataFromJson(json_t *rootJ) {
	}

	///////////////////////
	// Events
	///////////////////////

	// All of these events are thread-safe with process().

	struct AddEvent {};
	virtual void onAdd(const AddEvent &e) {
		// Call deprecated event method by default
		onAdd();
	}

	struct RemoveEvent {};
	virtual void onRemove(const RemoveEvent &e) {
		// Call deprecated event method by default
		onRemove();
	}

	struct BypassEvent {};
	virtual void onBypass(const BypassEvent &e) {
	}

	struct UnBypassEvent {};
	virtual void onUnBypass(const UnBypassEvent &e) {
	}

	struct PortChangeEvent {
		bool connecting;
		Port::Type type;
		int portId;
	};
	virtual void onPortChange(const PortChangeEvent &e) {
	}

	struct SampleRateChangeEvent {
		float sampleRate;
		float sampleTime;
	};
	virtual void onSampleRateChange(const SampleRateChangeEvent &e) {
		// Call deprecated event method by default
		onSampleRateChange();
	}

	struct ExpanderChangeEvent {
		uint8_t side;
	};
	virtual void onExpanderChange(const ExpanderChangeEvent &e) {
	}

	struct ResetEvent {};
	virtual void onReset(const ResetEvent &e);

	struct RandomizeEvent {};
	virtual void onRandomize(const RandomizeEvent &e);

	struct SaveEvent {};
	virtual void onSave(const SaveEvent &e) {
	}

	struct SetMasterEvent {};
	virtual void onSetMaster(const SetMasterEvent &e) {
	}

	struct UnsetMasterEvent {};
	virtual void onUnsetMaster(const UnsetMasterEvent &e) {
	}

	virtual void onAdd() {
	}
	virtual void onRemove() {
	}
	virtual void onReset() {
	}
	virtual void onRandomize() {
	}
	virtual void onSampleRateChange() {
	}

	bool isBypassed();

	PRIVATE void setBypassed(bool bypassed);
	PRIVATE const float *meterBuffer();
	PRIVATE int meterLength();
	PRIVATE int meterIndex();
	PRIVATE void doProcess(const ProcessArgs &args);
	PRIVATE static void jsonStripIds(json_t *rootJ);
	PRIVATE void setExpanderModule(Module *module, uint8_t side);

	// MetaModule: patch loader calls this to load state from a string (typically json)
	void load_state(std::string_view state_data) override;

	// MetaModule: patch loader calls this to save state to a string (typically json)
	std::string save_state() override;

	// MetaModule: audio engine calls this on all modules
	void set_samplerate(float sr) override;

	void show_graphic_display(int display_id, std::span<uint32_t> pix_buffer, unsigned width, lv_obj_t *) override;

	bool draw_graphic_display(int display_id) override;

	void hide_graphic_display(int display_id) override;
};

} // namespace engine
} // namespace rack
