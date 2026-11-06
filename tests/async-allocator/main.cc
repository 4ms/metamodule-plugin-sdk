
#include <rack.hpp>

#include "threads/async_thread.hh"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace rack;

Plugin *pluginInstance;

namespace
{

struct AsyncAllocator : Module {
	AsyncAllocator() {
		config(0, 0, 0, 0);
		printf("AsyncAllocator created\n");
		thread.start();
	}

	~AsyncAllocator() override {
		thread.stop();
	}

	void process(const ProcessArgs &) override {
	}

	void service() {
		// TODO: allocate a lot?
		std::string x;
		for (unsigned i = 0; i < 200; i++)
			x.append(std::rand() & 0x7f);
	}

	MetaModule::AsyncThread thread{this, [this]() { service(); }};
};

struct AsyncAllocatorWidget : ModuleWidget {
	AsyncAllocatorWidget(rack::Module *module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panel.svg")));
	}
};

} // namespace

auto modelAsyncAlloc = createModel<AsyncAllocator, AsyncAllocatorWidget>("asyncalloc");

void init(Plugin *p) {
	pluginInstance = p;
	p->addModel(modelAsyncAlloc);
}
