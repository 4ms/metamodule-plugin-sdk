#pragma once
#include <common.hpp>

namespace rack::engine
{

struct Module;

struct LightInfo {
	Module *module = nullptr;
	int lightId = -1;

	std::string name;

	std::string description;

	virtual ~LightInfo() = default;

	virtual std::string getName();
	virtual std::string getDescription();
};

} // namespace rack::engine
