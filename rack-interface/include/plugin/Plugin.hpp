#pragma once
#include <string>
#include <vector>

namespace rack::plugin
{

struct Model;
struct json_t;

struct Plugin {
	std::vector<Model *> models;

	std::string path;

	void *handle = nullptr;

	std::string slug; //brand slug

	std::string version;
	std::string license;
	std::string name;
	std::string brand;
	std::string description;
	std::string author;
	std::string authorEmail;
	std::string authorUrl;
	std::string pluginUrl;
	std::string manualUrl;
	std::string sourceUrl;
	std::string donateUrl;
	std::string changelogUrl;
	double modifiedTimestamp{};

	Plugin();
	Plugin(std::string slug);

	~Plugin();
	void addModel(Model* model);
	Model* getModel(const std::string& slug);
	void fromJson(json_t* rootJ);
	void modulesFromJson(json_t* rootJ);
	std::string getBrand();


};

} // namespace rack::plugin

