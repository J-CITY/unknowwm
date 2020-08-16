#include "client.h"
#include "config.h"

using namespace UW;

void Client::decorateWin(Display* display, Window rootWin, Config& config, unsigned int winFocus, unsigned int decorateWinFocus) {
	decorate = XCreateSimpleWindow(display, rootWin, 0, 0, 100, 100, config.BORDER_WIDTH,
		winFocus, decorateWinFocus);
	isDecorated = true;
	XWindowAttributes wa;
	if (XGetWindowAttributes(display, win, &wa)) {
		auto _w = wa.width;
		auto _h = wa.height;
		XMoveResizeWindow(display, decorate, wa.x, wa.y, wa.width, wa.height);
		moveResizeLocal(wa.x, wa.y, wa.width, wa.height, config, display);
		
		XWindowAttributes testWa;
		XGetWindowAttributes(display, win, &testWa);
		if (_w -2*(config.DECORATE_BORDER_WIDTH)- 2*(config.BORDER_WIDTH) - ((config.SHOW_TITLE && (config.TITLE_POSITION == TITLE_RIGHT || config.TITLE_POSITION == TITLE_LEFT)) ? config.TITLE_HEIGHT:0) < testWa.width || 
			_h -2*(config.DECORATE_BORDER_WIDTH)- 2*(config.BORDER_WIDTH) - ((config.SHOW_TITLE && (config.TITLE_POSITION == TITLE_UP || config.TITLE_POSITION == TITLE_DOWN)) ? config.TITLE_HEIGHT:0)  < testWa.height) {
			//XMoveResizeWindow(display, c->decorate, testWa.x, wa.y, wa.width, wa.height);
			int titleup = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_UP) ? config.TITLE_HEIGHT:0;
			int titledown = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_DOWN) ? config.ATTACH_ASIDE:0;
			int titleleft = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_LEFT) ? config.TITLE_HEIGHT:0;
			int titleright = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_RIGHT) ? config.TITLE_HEIGHT:0;
			int titleh = config.SHOW_TITLE ? config.TITLE_HEIGHT:0;
			XMoveResizeWindow(display, decorate, 
				testWa.x - config.BORDER_WIDTH - config.DECORATE_BORDER_WIDTH - titleleft, 
				testWa.y - config.BORDER_WIDTH - config.DECORATE_BORDER_WIDTH - titleup, 
				testWa.width+2*(config.DECORATE_BORDER_WIDTH) + ((titleright || titleleft)?titleh:0), 
				testWa.height+2*(config.DECORATE_BORDER_WIDTH) + ((titleup || titledown)?titleh:0));
		}
	}
}

void Client::decorationsDestroy(Display* display) {
	XUnmapWindow(display, decorate);
	XDestroyWindow(display, decorate);
	isDecorated = false;
}

void Client::moveResizeLocal(int x, int y, int w, int h, Config& config, Display* display) {
	int titleup = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_UP) ? config.TITLE_HEIGHT:0;
	int titledown = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_DOWN) ? config.TITLE_HEIGHT:0;
	int titleleft = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_LEFT) ? config.TITLE_HEIGHT:0;
	int titleright = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_RIGHT) ? config.TITLE_HEIGHT:0;
	int titleh = config.SHOW_TITLE ? config.TITLE_HEIGHT:0;
	XMoveResizeWindow(display, win, 
		x + config.BORDER_WIDTH + config.DECORATE_BORDER_WIDTH + titleleft, 
		y + config.BORDER_WIDTH + config.DECORATE_BORDER_WIDTH + titleup, 
		w-2*(config.DECORATE_BORDER_WIDTH) - ((titleright || titleleft)?titleh:0), 
		h-2*(config.DECORATE_BORDER_WIDTH) - ((titleup || titledown)?titleh:0));
}

void Client::moveLocal(int x, int y, Config& config, Display* display) {
	int titleup = config.SHOW_TITLE && config.TITLE_POSITION == TITLE_UP ? config.TITLE_HEIGHT:0;
	int titleleft = config.SHOW_TITLE && config.TITLE_POSITION == TITLE_LEFT ? config.TITLE_HEIGHT:0;
	XMoveWindow(display, win, 
		x + config.BORDER_WIDTH + config.DECORATE_BORDER_WIDTH + titleleft, 
		y + config.BORDER_WIDTH + config.DECORATE_BORDER_WIDTH + titleup);
}

void Client::resizeLocal(int w, int h, Config& config, Display* display) {
	int titleup = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_UP) ? config.TITLE_HEIGHT:0;
	int titledown = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_DOWN) ? config.TITLE_HEIGHT:0;
	int titleleft = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_LEFT) ? config.TITLE_HEIGHT:0;
	int titleright = (config.SHOW_TITLE && config.TITLE_POSITION == TITLE_RIGHT) ? config.TITLE_HEIGHT:0;
	int titleh = config.SHOW_TITLE ? config.TITLE_HEIGHT:0;
	XResizeWindow(display, win, 
		w-2*(config.DECORATE_BORDER_WIDTH) - ((titleright!=0 || titleleft!=0)?titleh:0), 
		h-2*(config.DECORATE_BORDER_WIDTH) - ((titleup!=0 || titledown!=0)?titleh:0));
}
