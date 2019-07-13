#ifndef UTILS_H
#define UTILS_H

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

#include "monitor.h"

enum { RESIZE, MOVE };
//Modes
enum { V_STACK_LEFT, V_STACK_RIGHT, H_STACK_UP, H_STACK_DOWN, MONOCLE, GRID, FLOAT, FIBONACCI, DOUBLE_STACK_VERTICAL, MODES };
enum { WM_PROTOCOLS, WM_DELETE_WINDOW, WM_COUNT };
enum { NET_SUPPORTED, NET_FULLSCREEN, NET_WM_STATE, NET_ACTIVE, NET_COUNT };
enum { TITLE_UP, TITLE_DOWN, TITLE_LEFT, TITLE_RIGHT };
#include <iostream>
struct Argument {
	std::vector<std::string> com;
	std::vector<int> intArr;
	int i;
	void *v;

	Argument(int _i): i(_i){}
	//Argument(std::vector<char*> &_com): com(_com){}
	Argument(void *_v): v(_v){}
	Argument(std::vector<int> &_intArr): intArr(_intArr){}
	Argument(){}
	Argument(std::vector<std::string> &_com) {
		for (auto i = 0; i < _com.size(); ++i) {
			com.push_back(_com[i]);
		}
		//com.push_back(nullptr);
		std::cout << com[0] << " $$\n";
	}
};

typedef void (*KeyFunc)(const Argument *);
struct Key {
	unsigned int mod;
	KeySym keysym;
	std::string func="";
	//KeyFunc func;
	const Argument arg;

	Key(unsigned int _mod, KeySym _keysym, std::string _func,
		const Argument _arg):mod(_mod), keysym(_keysym), func(_func), arg(_arg){}
};

typedef void (*ButtonFunc)(const Argument *);
struct Button {
	unsigned int mask, button;
	std::string func="";
	//ButtonFunc func;
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
		const bool isFollow, const bool  isFloating): appClass(appClass), monitor(monitor), desktop(desktop), isFollow(isFollow), isFloating(isFloating){}
};

struct Dl{
	int mode;
	int masz;
	bool sbar;
	Dl(int _mode, int _masz, bool _sbar): mode(_mode), masz(_masz), sbar(_sbar){}
	Dl(const Dl &_dl) {
		mode = _dl.mode;
		masz = _dl.masz;
		sbar = _dl.sbar;
	}
};

struct Ml {
	int m;
	int d;
	Dl dl;
	Ml(int m, int d, Dl dl): m(m), d(d), dl(dl) {}
};

#endif
