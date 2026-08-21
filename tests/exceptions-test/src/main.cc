// Test plugin for C++ exception and stream support in the MetaModule plugin
// SDK.
//
// Runs a battery of throw/catch and iostream tests at plugin load (init) and
// whenever a module instance is created, printing PASS/FAIL for each to the
// console. get_output(0) reports the result: number of passed tests if all
// passed, negative number of failures otherwise.

#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/element_info.hh"
#include "CoreModules/register_module.hh"

#include <array>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <libgen.h>
#include <memory>
#include <new>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <vector>

namespace
{

struct TestError : std::runtime_error {
	using std::runtime_error::runtime_error;
};

// Counts destructor executions so tests can verify cleanup during unwinding
struct Tracked {
	int &counter;
	explicit Tracked(int &c)
		: counter{c} {
	}
	~Tracked() {
		counter++;
	}
};

[[gnu::noinline]] void throw_deep(int depth, int &dtors) {
	Tracked t{dtors};
	if (depth > 0)
		throw_deep(depth - 1, dtors);
	else
		throw TestError{"deep"};
}

int run_exception_tests() {
	int passed = 0;
	int failed = 0;
	auto check = [&](bool ok, const char *name) {
		printf("[exc-test] %-32s %s\n", name, ok ? "PASS" : "FAIL");
		ok ? passed++ : failed++;
	};

	{
		bool ok = false;
		try {
			throw 42;
		} catch (int v) {
			ok = (v == 42);
		}
		check(ok, "throw/catch int");
	}

	{
		bool ok = false;
		try {
			throw 3.5;
		} catch (int) {
		} catch (double d) {
			ok = (d == 3.5);
		}
		check(ok, "catch selects matching type");
	}

	{
		bool ok = false;
		try {
			throw TestError{"abc"};
		} catch (std::exception &e) {
			ok = (std::string_view{e.what()} == "abc");
		}
		check(ok, "catch derived via base&, what()");
	}

	{
		bool ok = false;
		std::vector<int> v{1, 2, 3};
		try {
			[[maybe_unused]] auto _ = v.at(99);
		} catch (std::out_of_range &) {
			ok = true;
		}
		check(ok, "vector::at throws out_of_range");
	}

	{
		int dtors = 0;
		bool ok = false;
		try {
			throw_deep(5, dtors);
		} catch (TestError &) {
			ok = (dtors == 6);
		}
		check(ok, "dtors run during unwind");
	}

	{
		bool ok = false;
		try {
			try {
				throw TestError{"rethrown"};
			} catch (...) {
				throw;
			}
		} catch (std::runtime_error &e) {
			ok = (std::string_view{e.what()} == "rethrown");
		}
		check(ok, "rethrow from catch(...)");
	}

	{
		bool ok = false;
		std::exception_ptr ep;
		try {
			throw TestError{"via ptr"};
		} catch (...) {
			ep = std::current_exception();
		}
		if (ep) {
			try {
				std::rethrow_exception(ep);
			} catch (TestError &e) {
				ok = (std::string_view{e.what()} == "via ptr");
			}
		}
		check(ok, "exception_ptr rethrow");
	}

	{
		int seen = -1;
		bool ok = false;
		struct Probe {
			int &out;
			~Probe() {
				out = std::uncaught_exceptions();
			}
		};
		try {
			Probe p{seen};
			throw TestError{"count"};
		} catch (TestError &) {
			ok = (seen == 1);
		}
		check(ok, "uncaught_exceptions in dtor");
	}

	{
		bool ok = false;
		try {
			try {
				throw 1;
			} catch (int) {
				throw TestError{"from handler"};
			}
		} catch (TestError &) {
			ok = true;
		}
		check(ok, "throw from catch handler");
	}

	{
		bool ok = false;
		try {
			throw std::make_unique<int>(7);
		} catch (std::unique_ptr<int> &p) {
			ok = (*p == 7);
		}
		check(ok, "throw move-only object");
	}

	printf("[exc-test] %d passed, %d failed\n", passed, failed);
	return failed ? -failed : passed;
}

int run_oom_tests() {
	int passed = 0;
	int failed = 0;
	auto check = [&](bool ok, const char *name) {
		printf("[oom-test] %-32s %s\n", name, ok ? "PASS" : "FAIL");
		ok ? passed++ : failed++;
	};

	constexpr size_t huge = size_t{1} << 30; // 1 GB: larger than any possible heap

	{
		// malloc/new must honor the platform ABI contract (max_align_t = 8 on
		// arm32): GCC emits NEON stores with :64 alignment hints for buffers
		// it knows came from malloc, which trap on less-aligned addresses.
		// (Regression test: the arena's TLSF originally returned 4-aligned.)
		constexpr size_t need = alignof(std::max_align_t);
		bool ok = true;
		for (size_t sz : {1u, 2u, 3u, 5u, 8u, 13u, 20u, 100u, 1000u, 4097u}) {
			void *m = std::malloc(sz);
			auto *n = new char[sz];
			if (reinterpret_cast<uintptr_t>(m) % need || reinterpret_cast<uintptr_t>(n) % need)
				ok = false;
			std::free(m);
			delete[] n;
		}
		check(ok, "malloc/new 8-byte aligned");
	}

	{
		auto *p = new (std::nothrow) char[huge];
		check(p == nullptr, "nothrow new returns nullptr");
		delete[] p;
	}

	{
		// A single unservable request throws without draining the heap first
		bool ok = false;
		try {
			auto *p = new char[huge];
			// Defeat allocation elision: with no observable use of p, GCC
			// removes the new/delete pair entirely and nothing ever throws
			asm volatile("" : : "r"(p) : "memory");
			delete[] p;
		} catch (std::bad_alloc &) {
			ok = true;
		}
		check(ok, "huge new throws bad_alloc");
	}

	{
		bool ok = false;
		try {
			std::vector<char> v;
			v.resize(huge);
		} catch (std::bad_alloc &) {
			ok = true;
		}
		check(ok, "vector resize throws bad_alloc");
	}

	{
		// The UnfilteredVolume1 pattern: creep up in big steps until the heap
		// is exhausted, then recover. Throwing while the heap is genuinely full
		// also exerces __cxa_allocate_exception's emergency pool.
		// CAUTION: between the last successful chunk and the frees below, the
		// heap is transiently empty; an unlucky concurrent allocation elsewhere
		// in the firmware will fail during that window.
		constexpr size_t chunk_size = 16 * 1024 * 1024;
		std::array<char *, 64> chunks{};
		size_t n = 0;
		bool threw = false;
		try {
			for (; n < chunks.size(); n++)
				chunks[n] = new char[chunk_size];
		} catch (std::bad_alloc &) {
			threw = true;
		}
		printf("[oom-test] (allocated %zu chunks of %zu MB before bad_alloc)\n", n, chunk_size / (1024 * 1024));
		for (auto *p : chunks)
			delete[] p;
		check(threw, "allocating until exhaustion throws");

		// The heap must still be fully usable after being run dry
		bool ok = false;
		try {
			auto *p = new char[chunk_size];
			p[0] = 1;
			p[chunk_size - 1] = 2;
			ok = (p[0] == 1 && p[chunk_size - 1] == 2);
			delete[] p;
		} catch (std::bad_alloc &) {}
		check(ok, "heap usable after exhaustion");
	}

	printf("[oom-test] %d passed, %d failed\n", passed, failed);
	return failed ? -failed : passed;
}

int run_stream_tests() {
	int passed = 0;
	int failed = 0;
	auto check = [&](bool ok, const char *name) {
		printf("[stream-test] %-32s %s\n", name, ok ? "PASS" : "FAIL");
		ok ? passed++ : failed++;
	};

	{
		std::ostringstream oss;
		oss << "x=" << 42 << " y=" << std::fixed << std::setprecision(2) << 3.14159 << " hex=" << std::hex << 255;
		check(oss.str() == "x=42 y=3.14 hex=ff", "ostringstream formatting");
	}

	{
		std::istringstream iss{"123 4.5 hello"};
		int i{};
		double d{};
		std::string s;
		iss >> i >> d >> s;
		check(i == 123 && d == 4.5 && s == "hello" && !iss.fail(), "istringstream parsing");
	}

	{
		std::istringstream iss{"notanumber"};
		int i = 0;
		iss >> i;
		check(iss.fail(), "failbit set on bad parse");
	}

	{
		std::istringstream iss{"line one\nline two\n"};
		std::string a, b;
		std::getline(iss, a);
		std::getline(iss, b);
		check(a == "line one" && b == "line two", "getline");
	}

	{
		std::stringstream ss;
		ss << -17 << ' ' << "words";
		int i{};
		std::string w;
		ss >> i >> w;
		check(i == -17 && w == "words", "stringstream round-trip");
	}

	{
		bool ok = false;
		std::istringstream iss{"abc"};
		iss.exceptions(std::ios::failbit);
		try {
			int i{};
			iss >> i;
		} catch (std::ios_base::failure &) {
			ok = true;
		} catch (...) {}
		check(ok, "stream throws ios_base::failure");
	}

	{
		std::cout << "[stream-test] hello from std::cout" << std::endl;
		std::cerr << "[stream-test] hello from std::cerr\n";
		check(std::cout.good(), "cout/cerr writable");
	}

	{
		std::ifstream f{"nonexistent-file-xyz.txt"};
		check(!f.is_open() && f.fail(), "ifstream missing file fails cleanly");
	}

	{
		// Filesystem may not be writable from a plugin's working directory;
		// pass if the file round-trips or if the open fails cleanly. Report
		// which stage failed: a read-back-open failure after a successful
		// write+close has been observed when running from module-creation
		// context (vs plugin load) and is a host fs question, not a streams
		// problem.
		std::ofstream out{"usb:/exc-test-scratch.txt"};
		if (out.is_open()) {
			out << "roundtrip " << 99;
			out.close();
			std::ifstream in{"usb:/exc-test-scratch.txt"};
			if (in.is_open()) {
				std::string word;
				int num{};
				in >> word >> num;
				// note: the scratch file is left behind; the host does not
				// provide an unlink/remove syscall
				check(word == "roundtrip" && num == 99, "fstream write/read round-trip");
			} else {
				check(false, "read-back open after write+close");
			}
		} else {
			printf("[stream-test] (fs not writable, skipping fstream round-trip)\n");
			check(true, "ofstream unwritable fails cleanly");
		}
	}

	{ check(std::to_string(42) == "42" && std::stoi("-7") == -7, "to_string/stoi"); }

	{
		// std::random_device is widely used by ported VCV modules. On this
		// platform it falls back to a PRNG; the important thing is that it
		// links (a misconfigured libstdc++ makes it reference getentropy,
		// which the firmware does not provide) and produces values.
		std::random_device rd;
		std::mt19937 gen{rd()};
		std::uniform_int_distribution<int> dist{1, 6};
		int v = dist(gen);
		check(v >= 1 && v <= 6, "random_device/mt19937");
	}

	printf("[stream-test] %d passed, %d failed\n", passed, failed);
	return failed ? -failed : passed;
}

int run_libc_tests() {
	int passed = 0;
	int failed = 0;
	auto check = [&](bool ok, const char *name) {
		printf("[libc-test] %-32s %s\n", name, ok ? "PASS" : "FAIL");
		ok ? passed++ : failed++;
	};

	{
		// POSIX basename() (libgen.h) lives in newlib's libc/unix/, which ARM
		// toolchain builds exclude, so plugin-libc carries its own copy.
		// (Regression test: linking it went missing in the autotools rebuild.)
		char a[] = "/some/dir/file.txt";
		char b[] = "trailing/";
		check(std::string_view{basename(a)} == "file.txt" && std::string_view{basename(b)} == "trailing" &&
				  std::string_view{basename(nullptr)} == ".",
			  "basename");
	}

	{
		// stat() must bind to the firmware's export; newlib's syscall wrapper
		// chains to _stat_r/_stat which nothing provides, so plugin-libc must
		// not carry it. (Regression test: it crept back in the autotools
		// rebuild, breaking linking for any plugin that calls stat().)
		struct stat st{};
		check(stat("nonexistent-file-xyz.txt", &st) != 0, "stat missing file returns error");
	}

	{
		// aligned_alloc calls newlib's _memalign_r, which the SDK forwards to
		// the firmware's memalign (the firmware does not export _memalign_r)
		void *x = aligned_alloc(64, 128);
		check(x != nullptr, "aligned_alloc returns non-null");
		check((reinterpret_cast<uintptr_t>(x) & 63) == 0, "aligned_alloc result is aligned");
		if (x) {
			memset(x, 0xA5, 128);
			free(x);
		}
	}

	printf("[libc-test] %d passed, %d failed\n", passed, failed);
	return failed ? -failed : passed;
}

int run_all_tests() {
	int exc = run_exception_tests();
	int oom = run_oom_tests();
	int strm = run_stream_tests();
	int libc = run_libc_tests();
	if (exc < 0 || oom < 0 || strm < 0 || libc < 0)
		return (exc < 0 ? exc : 0) + (oom < 0 ? oom : 0) + (strm < 0 ? strm : 0) + (libc < 0 ? libc : 0);
	return exc + oom + strm + libc;
}

class ExcTestCore : public CoreProcessor {
public:
	ExcTestCore() {
		printf("[exc-test] module created: running exception + stream tests\n");
		result = run_all_tests();
	}

	void update() override {
	}
	void set_samplerate(float) override {
	}
	void set_param(int, float) override {
	}
	void set_input(int, float) override {
	}
	float get_output(int output_id) const override {
		return output_id == 0 ? static_cast<float>(result) : 0.f;
	}

private:
	int result = 0;
};

struct ExcTestInfo : MetaModule::ModuleInfoBase {
	static constexpr std::string_view slug{"ExcTest"};
	static constexpr std::string_view description{"C++ Exceptions Test"};
	static constexpr uint32_t width_hp = 4;
	static constexpr std::string_view png_filename{"ExceptionsTest/panel.png"};
};

// The two modules below are deliberately destructive: creating one exercises
// the firmware's handling of a plugin that dies. They do nothing at plugin
// load; add them from the GUI to trigger the failure on demand.

// Uncaught throw from a module constructor: unwinds out of all plugin frames,
// hits the firmware caller (no unwind tables visible to the plugin's
// unwinder), so the plugin's std::terminate runs and calls the host's abort().
class ThrowFatalCore : public ExcTestCore {
public:
	ThrowFatalCore() {
		printf("[exc-test] FATAL module created: throwing uncaught exception now\n");
		throw TestError{"deliberately uncaught"};
	}
};

// Uncaught bad_alloc from OOM in a module constructor: the UnfilteredVolume1
// failure replicated exactly -- allocate in big steps until the heap (or
// plugin arena) is exhausted, never catch.
class OomFatalCore : public ExcTestCore {
public:
	OomFatalCore() {
		printf("[exc-test] FATAL module created: allocating until OOM, not catching\n");
		std::vector<std::unique_ptr<char[]>> hoard;
		while (true)
			hoard.emplace_back(new char[2 * 1024 * 1024]);
	}
};

// Direct abort() from a module constructor: no exception involved, so this
// can never propagate across the boundary -- it exercises the host's
// abort-rescue (longjmp) fallback path, which is also what old-SDK plugins
// and terminate-without-throw deaths rely on.
class AbortFatalCore : public ExcTestCore {
public:
	AbortFatalCore() {
		printf("[exc-test] FATAL module created: calling abort() now\n");
		abort();
	}
};

struct AbortFatalInfo : MetaModule::ModuleInfoBase {
	static constexpr std::string_view slug{"ExcTestAbortFatal"};
	static constexpr std::string_view description{"Direct abort() Test (halts!)"};
	static constexpr uint32_t width_hp = 4;
	static constexpr std::string_view png_filename{"ExceptionsTest/panel.png"};
};

struct ThrowFatalInfo : MetaModule::ModuleInfoBase {
	static constexpr std::string_view slug{"ExcTestThrowFatal"};
	static constexpr std::string_view description{"Uncaught Throw Test (halts!)"};
	static constexpr uint32_t width_hp = 4;
	static constexpr std::string_view png_filename{"ExceptionsTest/panel.png"};
};

struct OomFatalInfo : MetaModule::ModuleInfoBase {
	static constexpr std::string_view slug{"ExcTestOomFatal"};
	static constexpr std::string_view description{"Uncaught OOM Test (halts!)"};
	static constexpr uint32_t width_hp = 4;
	static constexpr std::string_view png_filename{"ExceptionsTest/panel.png"};
};

} // namespace

extern "C" __attribute__((visibility("default"))) void init() {
	printf("[exc-test] plugin loaded: running exception + stream tests\n");
	run_all_tests();
	MetaModule::register_module<ExcTestCore, ExcTestInfo>("ExceptionsTest");
	MetaModule::register_module<ThrowFatalCore, ThrowFatalInfo>("ExceptionsTest");
	MetaModule::register_module<OomFatalCore, OomFatalInfo>("ExceptionsTest");
	MetaModule::register_module<AbortFatalCore, AbortFatalInfo>("ExceptionsTest");
}
