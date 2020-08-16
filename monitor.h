#pragma once

#include <vector>
#include <memory>

#include "desktop.h"


namespace UW {
struct Monitor {
	Monitor(int desktopsCount): desktopsCount(desktopsCount) {
		desktops.resize(desktopsCount);
	}
	~Monitor() {
		desktops.clear();
	}

	int desktopsCount = 0;
	int x, y, h, w, desktopCurId = 0, desktopPrevId = 0;
	std::vector<std::unique_ptr<Desktop>> desktops;

};
}

