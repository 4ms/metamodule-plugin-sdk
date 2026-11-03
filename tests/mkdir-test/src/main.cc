// Test plugin for creating directories:
//   mkdir()
//   rack::system::createDirectory() / createDirectories()
//
// When the module is created, it runs the checks on each of the SD Card and
// USB drive that's present. Each run gets a fresh directory, sdc:/mkdir-test-N/
// (or usb:/...), and the results are written to result.txt in a subdirectory
// of it, so reading that file on a computer confirms the directories are real.
// Results are also printed to the console.

#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/element_info.hh"
#include "CoreModules/register_module.hh"
#include <system.hpp>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <sys/stat.h>

namespace
{

struct Report {
	std::string text;
	unsigned failures = 0;

	void check(bool ok, std::string const &what) {
		auto line = std::string(ok ? "PASS " : "FAIL ") + what;
		printf("[mkdir-test] %s\n", line.c_str());
		text += line + "\n";
		if (!ok)
			failures++;
	}
};

// mkdir() returned -1 with this errno
bool fails_with(std::string const &path, int expected_errno) {
	errno = 0;
	return ::mkdir(path.c_str(), 0777) == -1 && errno == expected_errno;
}

// Returns false if the drive isn't present
bool test_drive(std::string const &vol, Report &r) {
	using namespace rack::system;

	// A fresh directory for this run: the first mkdir-test-N that mkdir() creates
	std::string base;
	for (int n = 1; n < 1000; n++) {
		auto path = vol + "mkdir-test-" + std::to_string(n);
		errno = 0;
		if (::mkdir(path.c_str(), 0777) == 0) {
			base = path;
			break;
		}
		if (errno == ENODEV)
			return false;
		if (errno != EEXIST)
			break;
	}
	r.check(!base.empty(), "mkdir() creates " + (base.empty() ? vol + "mkdir-test-N" : base));
	if (base.empty())
		return true;

	r.check(fails_with(base, EEXIST), "mkdir() on an existing dir fails with EEXIST");
	r.check(fails_with(base + "/no/such/parent", ENOENT), "mkdir() with a missing parent fails with ENOENT");
	r.check(fails_with(vol, EEXIST), "mkdir() on the volume root fails with EEXIST");

	r.check(createDirectory(base + "/one"), "createDirectory() creates a dir");
	r.check(!createDirectory(base + "/one"), "createDirectory() returns false when the dir exists");

	auto deep = base + "/a/b/c";
	r.check(createDirectories(deep), "createDirectories() creates nested dirs");
	r.check(!createDirectories(deep), "createDirectories() returns false when the dirs exist");
	r.check(createDirectories(base + "/a/b/d/"), "createDirectories() handles a trailing slash and existing parents");

	// Write the report into the deepest dir, which proves it exists on this drive
	auto report_path = deep + "/result.txt";
	FILE *f = fopen(report_path.c_str(), "w");
	bool written = f && fputs(r.text.c_str(), f) >= 0;
	if (f)
		written = (fclose(f) == 0) && written;
	r.check(written, "write " + report_path);

	return true;
}

class MkdirTestCore : public CoreProcessor {
public:
	MkdirTestCore() {
		Report r;
		r.check(fails_with("nor:/mkdir-test", EACCES), "mkdir() on the internal drive fails with EACCES");
		r.check(fails_with("ram:/mkdir-test", EACCES), "mkdir() on the RAM disk fails with EACCES");

		unsigned failures = r.failures;
		for (auto vol : {"sdc:/", "usb:/"}) {
			Report drive_report;
			drive_report.text = r.text; // include the drive-independent checks
			if (test_drive(vol, drive_report))
				failures += drive_report.failures;
			else
				printf("[mkdir-test] %s not present, skipped\n", vol);
		}

		printf("[mkdir-test] done: %u failures\n", failures);
	}

	void update() override {
	}

	void set_samplerate(float) override {
	}

	void set_param(int, float) override {
	}

	void set_input(int, float) override {
	}

	float get_output(int) const override {
		return 0.f;
	}
};

struct MkdirTestInfo : MetaModule::ModuleInfoBase {
	static constexpr std::string_view slug{"Mkdir"};
	static constexpr std::string_view description{"Mkdir Test"};
	static constexpr uint32_t width_hp = 4;
	static constexpr std::string_view png_filename{"MkdirTest/panel.png"};
};

} // namespace

extern "C" __attribute__((visibility("default"))) void init() {
	MetaModule::register_module<MkdirTestCore, MkdirTestInfo>("MkdirTest");
}
