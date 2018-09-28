#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xinerama.h>

enum { RESIZE, MOVE };
//Modes
enum { V_STACK_LEFT, V_STACK_RIGHT, H_STACK_UP, H_STACK_DOWN, MONOCLE, GRID, FLOAT, FIBONACCI, MODES };
enum { WM_PROTOCOLS, WM_DELETE_WINDOW, WM_COUNT };
enum { NET_SUPPORTED, NET_FULLSCREEN, NET_WM_STATE, NET_ACTIVE, NET_COUNT };
enum { TITLE_UP, TITLE_DOWN, TITLE_LEFT, TITLE_RIGHT };

struct Client {
	Client *next = nullptr;
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

	~Client() {
		delete next;
	}
};

struct Desktop {
	int mode, masterSize=0, firstStackSize=0, nm=0;
	int layoutId = 0;
	bool isHide = false;
	//Clients *head, *cur, *prev;

	std::vector<Client*> clients;
	int curClientId = -1;
	int prevClientId = -1;
	
	bool isBar;

	~Desktop() {
		for (auto it = clients.begin() ; it != clients.end(); ++it) {
			delete (*it);
		}
		clients.clear();
	}

	Client *GetCur() {
		if (clients.size() == 0 || curClientId < 0 || curClientId >= clients.size()) {
			return nullptr;
		}
		return clients[curClientId];
	}

	Client *GetHead() {
		if (clients.size() == 0) {
			return nullptr;
		}
		return clients[0];
	}

	Client *GetPrev() {
		if (clients.size() == 0 || prevClientId < 0 || prevClientId >= clients.size()) {
			return nullptr;
		}
		return clients[prevClientId];
	}
};

struct Monitor {
	int desktopsCount = 0;
	Monitor(int desktopsCount): desktopsCount(desktopsCount) {
		printf("DEBUGGGG %i", desktopsCount);
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

struct Argument {
	std::vector<char*> com;
	std::vector<int> intArr;
	int i;
	void *v;

	Argument(int _i): i(_i){}
	Argument(std::vector<char*> _com): com(_com){}
	Argument(void *_v): v(_v){}
	Argument(std::vector<int> _intArr): intArr(_intArr){}
	Argument(){}
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
	const char *appClass;
	const int monitor;
	const int desktop;
	const bool isFollow, isFloating;

	AppRule(const char *appClass, const int monitor,
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
