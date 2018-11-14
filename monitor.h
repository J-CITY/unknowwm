#include <vector>
#include "desktop.h"

struct Monitor {
	int desktopsCount = 0;
	Monitor(int desktopsCount): desktopsCount(desktopsCount) {
		desktops.resize(desktopsCount);
	}
	int x, y, h, w, desktopCurId = 0, desktopPrevId = 0;
	std::vector<Desktop*> desktops;

	~Monitor() {
		for (auto it = desktops.begin() ; it != desktops.end(); ++it) {
			delete (*it);
		}
		desktops.clear();
	}
};
