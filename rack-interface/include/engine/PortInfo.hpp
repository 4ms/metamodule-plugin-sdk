#pragma once
#include <common.hpp>
#include <engine/Port.hpp>

namespace rack::engine
{

struct Module;

struct PortInfo {
	Module *module = nullptr;
	Port::Type type = Port::INPUT;
	int portId = -1;

	std::string name;

	std::string description;

	virtual ~PortInfo() = default;

	virtual std::string getName();

	std::string getFullName();

	virtual std::string getDescription();
};

} // namespace rack::engine

