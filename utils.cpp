#include "utils.h"

using namespace UW;

long int UW::getColor(std::string _color, const int screen, Display* display) {
	const char* color = _color.c_str();
	XColor c;
	Colormap map = DefaultColormap(display, screen);
	if (!XAllocNamedColor(display, map, color, &c, &c)) {
		return 0;
	}
	return c.pixel;
}
