#pragma once

#include <map>
#include <memory>

#include <common.hpp>
#include <math.hpp>
#include <window/Svg.hpp>

#define GLEW_STATIC
#define GLEW_NO_GLU
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <nanovg.h>
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>

namespace rack::window
{

// Constructing these directly will load from the disk each time. Use the load() functions to load from disk and cache them as long as the shared_ptr is held.

struct Font {
	NVGcontext *vg{};
	int handle = -1;

	~Font();

	void loadFile(const std::string &filename, NVGcontext *vg);
	[[deprecated]] static std::shared_ptr<Font> load(const std::string &filename);
};

struct Image {
	NVGcontext *vg{};
	int handle = -1;

	~Image();

	void loadFile(const std::string &filename, NVGcontext *vg);
	[[deprecated]] static std::shared_ptr<Image> load(const std::string &filename);
};

struct Window {
	struct Internal;
	std::unique_ptr<Internal> internal;

	GLFWwindow *win = nullptr;
	NVGcontext *vg = nullptr;
	NVGcontext *fbVg = nullptr;
	float pixelRatio = 1.f;
	float windowRatio = 1.f;
	std::shared_ptr<Font> uiFont;

	Window();
	~Window();

	math::Vec getSize();
	void setSize(math::Vec size);
	PRIVATE void run();
	PRIVATE void step();
	PRIVATE void screenshot(const std::string &screenshotPath);
	PRIVATE void screenshotModules(const std::string &screenshotsDir, float zoom = 1.f);
	void close();
	void cursorLock();
	void cursorUnlock();
	bool isCursorLocked();
	int getMods();
	void setFullScreen(bool fullScreen);
	bool isFullScreen();
	double getMonitorRefreshRate();
	double getFrameTime();
	double getLastFrameDuration();
	double getFrameDurationRemaining();
	std::shared_ptr<Font> loadFont(const std::string &filename);
	std::shared_ptr<Image> loadImage(const std::string &filename);
	PRIVATE bool &fbDirtyOnSubpixelChange();
	PRIVATE int &fbCount();

	std::shared_ptr<Svg> loadSvg(const std::string &filename) {
		return Svg::load(filename);
	}
};

} // namespace rack::window
