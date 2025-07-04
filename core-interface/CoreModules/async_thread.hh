#pragma once
#include "CoreModules/CoreProcessor.hh"
#include "util/callable.hh"
#include <memory>

namespace MetaModule
{

class AsyncThread {
public:
	AsyncThread(CoreProcessor *module);
	AsyncThread(CoreProcessor *module, Callback &&action);

	void start();
	void start(Callback &&action);

	void stop();
	void run_once();

	bool is_enabled();

	~AsyncThread();

private:
	Callback action{};

	struct Internal;
	std::unique_ptr<Internal> internal;
};

} // namespace MetaModule
