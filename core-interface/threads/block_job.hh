#pragma once
#include "CoreModules/CoreProcessor.hh"
#include "util/callable.hh"
#include <memory>

namespace MetaModule
{

// Runs a module's block-based work (e.g. DSP that processes 32 samples at once)
// in the background, so it overlaps with the rest of the patch instead of
// stalling both cores on the one frame where it happens.
//
// The module calls submit() from update() when a block is ready, and must not
// touch anything `work` uses until the job is done. The engine makes sure the job
// has finished before the module's update() is called `frames` frames later.
// Typically a module double-buffers its audio and submits with frames = block size,
// which adds one block of latency.
class BlockJob {
public:
	BlockJob(CoreProcessor *module, Callback &&work);
	~BlockJob();

	// Starts the work in the background, to be finished before update() is
	// called `frames` frames from now. Returns false (and does nothing) if the
	// previous job hasn't finished yet.
	bool submit(unsigned frames);

	// True from submit() until the work has finished
	bool is_busy() const;

private:
	struct Internal;
	std::unique_ptr<Internal> internal;
};

} // namespace MetaModule
