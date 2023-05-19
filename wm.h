#pragma once

#include <memory>
#include <iostream>
#include <map>
#include <err.h>

#include "monitor.h"
#include "config.h"
#include "logger.h"

#define Length(x)       (sizeof(x)/sizeof(*x))

namespace UW {
class WindowManager {
public:
	typedef void (WindowManager::*ScriptFunction)(XEvent *e);
	typedef void (WindowManager::*ModeFunction)(int x, int y, int w, int h, Desktop *d);
	typedef void (WindowManager::*ActionFunction)(const Argument *arg);

	std::unique_ptr<Config> config;

	static bool isWmDetected;
	static int WMDetected(Display* display, XErrorEvent* e);
	static std::unique_ptr<WindowManager> Create();
	static int XError(Display* display, XErrorEvent* e);
	
	~WindowManager();
	void run();
	void desktopInfo();
	void init();
	void cleanup();

private:
	WindowManager(Display* display);

	Client* addWindow(Window w, Desktop* d);
	void deleteWindow(Window w);
	void focus(Client* c, Desktop* d, Monitor* m);
	void focusUrGent();
	
	void grabButtons(Client* c);
	void grabKeys();
	void removeClient(Client* c, Desktop* d, Monitor* m);
	bool winToClient(Window w, Client** c, Desktop** d, Monitor** m, bool isTitle=false);
	void gridMode(int x, int y, int w, int h, Desktop *d);
	void monocleMode(int x, int y, int w, int h, Desktop *d);
	void fullscreenMode(Client* c, Desktop* d, Monitor* m, bool fullscrn);
	void floatMode(int x, int y, int w, int h, Desktop* d);
	void stackMode(int x, int y, int w, int h, Desktop* d);
	void fibonacciMode(int x, int y, int w, int h, Desktop* d);
	void doubleStackVerticalMode(int x, int y, int w, int h, Desktop* d);
	static void Sigchld(int sig);
	void tile(Desktop* d, Monitor* m);

	void buttonPress(XEvent* e);
	void buttonPressTitle(XEvent* e);
	void configureRequest(XEvent* e);
	void clientMessage(XEvent* e);
	void destroyNotify(XEvent* e);
	void enterNotify(XEvent* e);
	void focusIn(XEvent* e);
	void keyPress(XEvent* e);
	void mapRequest(XEvent* e);
	void propertyNotify(XEvent* e);
	void unmapNotify(XEvent* e);
	void eventFromClient(XEvent* e);

	void focusWinById(const Argument* arg);
	void runFunc(std::string type, std::string funcStr, const Argument* arg);
	void toggleFullscreenClient(const Argument* arg);
	void toggleFloatClient(const Argument* arg);
	void moveDown(const Argument* arg);
	void moveUp(const Argument* arg);
	void nextWin(const Argument* arg);
	void prevWin(const Argument* arg);
	void killClient(const Argument* arg);
	void swapMaster(const Argument* arg);
	void togglePanel(const Argument* arg);
	void changeDesktop(const Argument* Argument);
	void changeMonitor(const Argument* Argument);
	void clientToDesktop(const Argument* Argument);
	void clientToMonitor(const Argument* Argument);
	void moveResize(const Argument* Aargrgument);
	void mouseMotion(const Argument* arg);
	void runCmd(const Argument* arg);
	void quit(const Argument* arg);
	void resizeMaster(const Argument* Argument);
	void resizeStack(const Argument* Argument);
	void nextDesktop(const Argument* Argument);
	void prevDesktop(const Argument* arg);
	void nextFilledDesktop(const Argument* Argument);
	void switchMode(const Argument* arg);
	void restartMonitors(const Argument* arg);
	void restart(const Argument* arg);
	void reloadConfig(const Argument* arg);

	inline unsigned int rootMask() {
		return SubstructureRedirectMask |
			ButtonPressMask | SubstructureNotifyMask | PropertyChangeMask;
	}

	inline bool isFloatOrFullscreen(Client* c) {
		if (c == nullptr) {
			return false;
		}
		return (c->isFull || c->isFloat || c->isTrans);
	}

	inline unsigned int buttonMask() {
		return ButtonPressMask|ButtonReleaseMask;
	}

	inline unsigned int cleanMask(int mask) {
		return (mask & ~(numlockmask | LockMask));
	}

	void hideClient(Client* c, Monitor* m);
	void hideCurClient(const Argument* arg);
	void hideAllClientOnDescktop(const Argument* arg);
	void changeDecorateBorder(const Argument* arg);
	void changeBorder(const Argument* arg);
	void changeGap(const Argument* arg);
	void addMaster(const Argument* arg);

	void changeLayout(const Argument* arg);

	bool isRunning = true;
	Display* display = nullptr;
	Window rootWin;
	std::vector<std::unique_ptr<Monitor>> monitors;
	int monitorCount, monitorId, retval;
	unsigned int numlockmask=0, winUnfocus=0, winFocus=0, 
		winInfocus=0, decorateWinUnfocus=0, decorateWinFocus=0, 
		decorateWinInfocus=0, titleTextColor=0;
	Atom wmatoms[WM_COUNT], netatoms[NET_COUNT];

	Atom unknowwm_client_event;

	std::map<int, ScriptFunction> events = {
		{KeyPress,        &WindowManager::keyPress},
		{EnterNotify,     &WindowManager::enterNotify},
		{MapRequest,      &WindowManager::mapRequest},
		{ClientMessage,   &WindowManager::clientMessage},
		{ButtonPress,     &WindowManager::buttonPress},
		{DestroyNotify,   &WindowManager::destroyNotify},
		{UnmapNotify,     &WindowManager::unmapNotify},
		{PropertyNotify,  &WindowManager::propertyNotify},
		{ConfigureRequest,&WindowManager::configureRequest},
		{FocusIn,         &WindowManager::focusIn}
	};

	std::map<int, ModeFunction> layout = {
		{V_STACK_LEFT,         &WindowManager::stackMode},
		{V_STACK_RIGHT,        &WindowManager::stackMode},
		{H_STACK_UP,           &WindowManager::stackMode},
		{H_STACK_DOWN,         &WindowManager::stackMode},
		{GRID,                 &WindowManager::gridMode},
		{MONOCLE,              &WindowManager::monocleMode},
		{FIBONACCI,            &WindowManager::fibonacciMode},
		{FLOAT_MODE,           &WindowManager::floatMode},
		{DOUBLE_STACK_VERTICAL,&WindowManager::doubleStackVerticalMode}
	};
	
	std::map<std::string, ActionFunction> runFuncMap = {
		{ACTION_TOGGLE_PANEL, &WindowManager::togglePanel},
		{ACTION_SWAP_MASTER, &WindowManager::swapMaster},
		{ACTION_QUIT, &WindowManager::quit},
		{ACTION_RUN_CMD, &WindowManager::runCmd},
		{ACTION_KILL_CILENT, &WindowManager::killClient},
		{ACTION_NEXT_WIN, &WindowManager::nextWin},
		{ACTION_PREW_WIN, &WindowManager::prevWin},
		{ACTION_MOVE_RESIZE, &WindowManager::moveResize},
		{ACTION_SWITCH_MODE, &WindowManager::switchMode},
		{ACTION_RESIZE_MASTER, &WindowManager::resizeMaster},
		{ACTION_RESIZE_STACK, &WindowManager::resizeStack},
		{ACTION_MOVE_DOWN, &WindowManager::moveDown},
		{ACTION_MOVE_UP, &WindowManager::moveUp},
		{ACTION_NEXT_DESKTOP, &WindowManager::nextDesktop},
		{ACTION_NEXT_FILLED_DESKTOP, &WindowManager::nextFilledDesktop},
		{ACTION_PREV_DESKTOP, &WindowManager::prevDesktop},
		{ACTION_CLIENT_TO_DESKTOP, &WindowManager::clientToDesktop},
		{ACTION_TOGGLE_FLOAT_CLIENT, &WindowManager::toggleFloatClient},
		{ACTION_TOGGLE_FULLSCREEN_CLIENT, &WindowManager::toggleFullscreenClient},
		{ACTION_CHANGE_DECORATE_BORDER, &WindowManager::changeDecorateBorder},
		{ACTION_CHANGE_BORDER, &WindowManager::changeBorder},
		{ACTION_CHANGE_GAP, &WindowManager::changeGap},
		{ACTION_ADD_MASTER, &WindowManager::addMaster},
		{ACTION_HIDE_CUR_CLIENT, &WindowManager::hideCurClient},
		{ACTION_HIDE_ALL_CLIENT_ON_DESKTOP, &WindowManager::hideAllClientOnDescktop},
		{ACTION_CHANGE_DESKTOP, &WindowManager::changeDesktop},
		{ACTION_CHANGE_LAYOUT, &WindowManager::changeLayout},
		{ACTION_CHANGE_MONITOR, &WindowManager::changeMonitor},
		{ACTION_CLIENT_TO_MONITOR, &WindowManager::clientToMonitor},
		{ACTION_RESTART, &WindowManager::restart},
		{ACTION_RESTART_MONITORS, &WindowManager::restartMonitors},
	};
	
	struct MoveEvent {
		int x = 0;
		int y = 0;
		int curx = 0;
		int cury = 0;
		Client* c;
		float speed = 1.0f;
	};
	std::unordered_map<Window, MoveEvent> moveEvents;
	struct ResizeEvent {
		int w = 0;
		int h = 0;
		int curw = 0;
		int curh = 0;
		Client* c;
		float speed = 1.0f;
	};
	std::unordered_map<Window, ResizeEvent> resizeEvents;
};
};

