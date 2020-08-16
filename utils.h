#pragma once
#include <vector>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xft/Xft.h>

namespace UW {

enum { RESIZE, MOVE };
//Modes
enum { V_STACK_LEFT, V_STACK_RIGHT, H_STACK_UP, H_STACK_DOWN, MONOCLE, GRID, FLOAT_MODE, FIBONACCI, DOUBLE_STACK_VERTICAL, MODES };
enum { WM_PROTOCOLS, WM_DELETE_WINDOW, WM_COUNT };
enum { NET_SUPPORTED, NET_FULLSCREEN, NET_WM_STATE, NET_ACTIVE, NET_COUNT };
enum { TITLE_UP, TITLE_DOWN, TITLE_LEFT, TITLE_RIGHT };

struct Rect {
	int x=0, y=0;
	unsigned int w=0, h=0;
};

struct Argument {
	std::vector<std::string> com;
	std::vector<int> intArr;
	int i;
	void *v;

	Argument(int _i): i(_i) {}
	Argument(void* _v): v(_v) {}
	Argument(std::vector<int> &_intArr): intArr(_intArr) {}
	Argument(){}
	Argument(std::vector<std::string> &_com) {
		for (auto i = 0; i < _com.size(); ++i) {
			com.push_back(_com[i]);
		}
	}
};

typedef void (*KeyFunc)(const Argument *);
struct Key {
	unsigned int mod;
	KeySym keysym;
	std::string func = "";
	const Argument arg;

	Key(unsigned int _mod, KeySym _keysym, std::string _func,
		const Argument _arg):mod(_mod), keysym(_keysym), func(_func), arg(_arg) {}
};

typedef void (*ButtonFunc)(const Argument *);
struct Button {
	unsigned int mask, button;
	std::string func = "";
	const Argument arg;
	Button(unsigned int _mask, unsigned int _button,
		std::string _func,
		const Argument _arg): mask(_mask), button(_button), func(_func), arg(_arg) {}
};

struct AppRule {
	const std::string appClass;
	const int monitor;
	const int desktop;
	const bool isFollow, isFloating;

	AppRule(const std::string appClass, const int monitor,
		const int desktop,
		const bool isFollow, const bool  isFloating): 
			appClass(appClass),
			monitor(monitor),
			desktop(desktop),
			isFollow(isFollow),
			isFloating(isFloating) {}
};

long int getColor(std::string _color, const int screen, Display* display);
}

