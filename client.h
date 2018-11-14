#include <string>

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

	~Client() {}
};
