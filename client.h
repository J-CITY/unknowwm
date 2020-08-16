#pragma once

#include <string>
#include "utils.h"

namespace UW {
class Config;

struct Client {
	bool isUrgn, isFull, isFloat, isTrans=false;
	Window win;
	Window decorate;
	bool isDecorated = false;
	int id = -1;
	std::string title = "";
	bool isUnmap = false;

	int hideX = 0;
	int hideY = 0;
	bool isHide = false;
	bool isIgnore = false;

	//TODO:
	//Rect rect;

	~Client() = default;
	void decorateWin(Display* display, Window rootWin, Config& config, unsigned int winFocus, unsigned int decorateWinFocus);
	void decorationsDestroy(Display* display);
	void moveResizeLocal(int x, int y, int w, int h, Config& config, Display* display);
	void moveLocal(int x, int y, Config& config, Display* display);
	void resizeLocal(int w, int h, Config& config, Display* display);
};
}
