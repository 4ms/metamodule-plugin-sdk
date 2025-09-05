#pragma once
#include "CoreModules/CoreProcessor.hh"
#include <engine/Light.hpp>
#include <engine/Param.hpp>
#include <engine/ParamQuantity.hpp>
#include <engine/Port.hpp>
#include <memory>
#include <vector>

struct ParamScale {
	float range;
	float offset;
};

struct VCVModuleWrapper : CoreProcessor {

	struct ProcessArgs {
		float sampleRate;
		float sampleTime;
		int64_t frame;
	};

	VCVModuleWrapper();
	~VCVModuleWrapper();

	void update() override;

	virtual void update(const ProcessArgs &args, bool bypassed) {
	}

	void set_samplerate(float rate) override;

	void set_param(int id, float val) override;

	void set_input(int input_id, float val) override;

	float get_param(int id) const override;

	float get_output(int output_id) const override;

	float get_led_brightness(int led_id) const override;

	void mark_all_inputs_unpatched() override;

	void mark_input_unpatched(int input_id) override;

	void mark_input_patched(int input_id) override;

	void mark_all_outputs_unpatched() override;

	void mark_output_unpatched(int output_id) override;

	void mark_output_patched(int output_id) override;

	std::vector<rack::engine::Param> params;
	std::vector<rack::engine::Input> inputs;
	std::vector<rack::engine::Output> outputs;
	std::vector<rack::engine::Light> lights;

	std::vector<rack::engine::ParamQuantity *> paramQuantities;

	ProcessArgs args{48000.f, 1.f / 48000.f, 0};
};
