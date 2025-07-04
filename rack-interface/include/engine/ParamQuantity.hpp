#pragma once
#include <vector>

#include <jansson.h>

#include <Quantity.hpp>
#include <engine/Param.hpp>

namespace rack::engine
{

struct Module;

struct ParamQuantity : Quantity {
	Module *module = nullptr;
	int paramId = -1;

	float minValue = 0.f;
	float maxValue = 1.f;
	float defaultValue = 0.f;

	std::string name;
	std::string unit;
	float displayBase = 0.f;
	float displayMultiplier = 1.f;
	float displayOffset = 0.f;
	int displayPrecision = 5;
	std::string description;

	bool resetEnabled = true;
	bool randomizeEnabled = true;
	bool smoothEnabled = false;
	bool snapEnabled = false;

	Param *getParam();

	void setValue(float value) override;
	float getValue() override;

	void setImmediateValue(float value);
	float getImmediateValue();

	float getMinValue() override;
	float getMaxValue() override;
	float getDefaultValue() override;
	float getDisplayValue() override;
	void setDisplayValue(float displayValue) override;
	std::string getDisplayValueString() override;
	void setDisplayValueString(std::string s) override;
	int getDisplayPrecision() override;
	std::string getLabel() override;
	std::string getUnit() override;
	void reset() override;
	void randomize() override;

	virtual std::string getDescription();

	virtual json_t *toJson();
	virtual void fromJson(json_t *rootJ);

	DEPRECATED void setSmoothValue(float value);
	DEPRECATED float getSmoothValue();

	// MetaModule: put labels in ParamQuantity
	// so we only have to store ParamQuantitys in Module
	std::vector<std::string> labels;
};

struct SwitchQuantity : ParamQuantity {
	std::string getDisplayValueString() override;
	void setDisplayValueString(std::string s) override;
};

} // namespace rack::engine

