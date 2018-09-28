#ifndef WM_H
#define WM_H

#include <memory>
#include "config.h"
#include <iostream>
#include <map>
#include <err.h>
#define Length(x)       (sizeof(x)/sizeof(*x))

class WindowManager {
public:
	typedef void (WindowManager::*ScriptFunction)(XEvent *e);
	typedef void (WindowManager::*ModeFunction)(int x, int y, int w, int h, Desktop *d);

	Config *config;

	static bool isWmDetected;
	static int WMDetected(Display* display, XErrorEvent* e);
	static std::unique_ptr<WindowManager> Create();
	static int XError(Display* display, XErrorEvent* e);
	
	~WindowManager();
	void Run();
	void DesktopInfo();
	void Init();
	void Cleanup();

private:
	WindowManager(Display* display);

	Client* AddWindow(Window w, Desktop *d);
	void DeleteWindow(Window w);
	void Focus(Client *c, Desktop *d, Monitor *m);
	void FocusUrGent();
	unsigned long GetColor(std::string _color, const int screen);
	void GrabButtons(Client *c);
	void GrabKeys();
	void KillClient(void);
	void PrevDesktop(void);
	void RemoveClient(Client *c, Desktop *d, Monitor *m);
	bool WinToClient(Window w, Client **c, Desktop **d, Monitor **m);
	void GridMode(int x, int y, int w, int h, Desktop *d);
	void MonocleMode(int x, int y, int w, int h, Desktop *d);
	void FullscreenMode(Client *c, Desktop *d, Monitor *m, bool fullscrn);
	void FloatMode(int x, int y, int w, int h, Desktop *d);
	void StackMode(int x, int y, int w, int h, Desktop *d);
	void FibonacciMode(int x, int y, int w, int h, Desktop *d);
	void MoveDown();
	void MoveUp();
	static void Sigchld(int sig);
	void SwapMaster();
	void Tile(Desktop *d, Monitor *m);
	void TogglePanel();
	void NextWin();
	void PrevWin();
	void ToggleFloatClient();
	void ToggleFullscreenClient();

	void buttonPress(XEvent *e);
	void configureRequest(XEvent *e);
	void clientMessage(XEvent *e);
	void destroyNotify(XEvent *e);
	void enterNotify(XEvent *e);
	void focusIn(XEvent *e);
	void keyPress(XEvent *e);
	void mapRequest(XEvent *e);
	void propertyNotify(XEvent *e);
	void unmapNotify(XEvent *e);

	void RunFunc(std::string type, std::string funcStr, const Argument *arg);
	void ChangeDesktop(const Argument *Argument);
	void ChangeMonitor(const Argument *Argument);
	void ClientToDesktop(const Argument *Argument);
	void ClientToMonitor(const Argument *Argument);
	void MoveResize(const Argument *Aargrgument);
	void MouseMotion(const Argument *arg);
	void RunCmd(const Argument *arg);
	void Quit(const Argument *arg);
	void ResizeMaster(const Argument *Argument);
	void ResizeStack(const Argument *Argument);
	void NextDesktop(const Argument *Argument);
	void NextFilledDesktop(const Argument *Argument);
	void SwitchMode(const Argument *arg);

	inline unsigned int RootMask() {
		return SubstructureRedirectMask |
			ButtonPressMask | SubstructureNotifyMask | PropertyChangeMask;
	}

	inline bool IsFloatOrFullscreen(Client *c) {
		if (c == nullptr) {
			return false;
		}
		return (c->isFull || c->isFloat || c->isTrans);
	}

	inline unsigned int ButtonMask() {
		return ButtonPressMask|ButtonReleaseMask;
	}

	inline unsigned int CleanMask(int mask) {
		return (mask & ~(numlockmask | LockMask));
	}

	void DecorationsDestroy(Client *c) {
		XUnmapWindow(display, c->decorate);
		XDestroyWindow(display, c->decorate);
		c->isDecorated = false;
	}

	void HideClient(Client *c, Monitor *m) {
		if (c == nullptr) {
			return;
		}
		auto win = config->SHOW_DECORATE && c->isDecorated ? (c)->decorate : (c)->win;
		
		XWindowAttributes wa;
		if (XGetWindowAttributes(display, win, &wa)) {
			
			if (c->isHide) {
				XMoveResizeWindow(display, win, c->hideX, c->hideY, wa.width, wa.height);
				if (c->isDecorated) {
					MoveResizeLocal(c->win, c->hideX, c->hideY, wa.width, wa.height);
				}
			} else {
				c->hideX = wa.x;
				c->hideY = wa.y;
				XMoveResizeWindow(display, win, - 2 * m->w, 0, wa.width, wa.height);
				if (c->isDecorated) {
					MoveResizeLocal(c->win, - 2 * m->w, 0, wa.width, wa.height);
				}
			}
			c->isHide = !c->isHide;
			
		}
	}

	void HideCurClient() {
		Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId];
		Client *c = d->GetCur();
		if (c == nullptr) {
			return;
		}
		HideClient(c, monitors[monitorId]);
	}

	void HideAllClientOnDescktop() {
		Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId];
		bool hide = true;
		for (auto _c = d->clients.begin(); _c != d->clients.end(); _c++) {
			if (!(*_c)->isHide) {
				hide = false;
				break;
			}
		}
		if (hide) {
			d->isHide = false;
			for (auto _c = d->clients.begin(); _c != d->clients.end(); _c++) {
				if ((*_c)->isHide) {
					HideClient((*_c), monitors[monitorId]);
				}
			}
		} else {
			d->isHide = true;
			for (auto _c = d->clients.begin(); _c != d->clients.end(); _c++) {
				if (!(*_c)->isHide) {
					HideClient((*_c), monitors[monitorId]);
				}
			}
		}
	}

	void ChangeDecorateBorder(const Argument *arg) {
		config->DECORATE_BORDER_WIDTH += arg->i;
		config->DECORATE_BORDER_WIDTH  = config->DECORATE_BORDER_WIDTH < 0 ? 0 : config->DECORATE_BORDER_WIDTH;
		config->DECORATE_BORDER_WIDTH  = config->DECORATE_BORDER_WIDTH > 100 ? 1000 : config->DECORATE_BORDER_WIDTH;
		Monitor *m = monitors[monitorId];
		Desktop *d = m->desktops[m->desktopCurId];
		Client  *c = d->GetCur();
		Tile(d, m);
		Focus(c, d, m);
	}

	void ChangeBorder(const Argument *arg) {
		config->BORDER_WIDTH += arg->i;
		config->BORDER_WIDTH  = config->BORDER_WIDTH < 0 ? 0 : config->BORDER_WIDTH;
		config->BORDER_WIDTH  = config->BORDER_WIDTH > 100 ? 1000 : config->BORDER_WIDTH;
		Monitor *m = monitors[monitorId];
		Desktop *d = m->desktops[m->desktopCurId];
		Client  *c = d->GetCur();
		Tile(d, m);
		Focus(c, d, m);
	}

	void ChangeGap(const Argument *arg) {
		config->USELESSGAP += arg->i;
		config->USELESSGAP  = config->USELESSGAP < 0 ? 0 : config->USELESSGAP;
		config->USELESSGAP  = config->USELESSGAP > 100 ? 1000 : config->USELESSGAP;
		Monitor *m = monitors[monitorId];
		Desktop *d = m->desktops[m->desktopCurId];
		Client  *c = d->GetCur();
		Tile(d, m);
		Focus(c, d, m);
	}

	void AddMaster(const Argument *arg) {
		Monitor *m =  monitors[monitorId];
		Desktop *d = m->desktops[monitors[monitorId]->desktopCurId];
		d->nm += arg->i;
		d->nm = d->nm <= 0 ? config->NMASTER : d->nm;
		Tile(d, m);
	}

	void ChangeLayout(const Argument *arg);
	void MoveResizeLocal(Window win , int x, int y, int w, int h);
	void MoveLocal(Window win, int x, int y);
	void ResizeLocal(Window win, int w, int h);

	bool isRunning = true;
	Display* display;
	Window rootWin;
	std::vector<Monitor*> monitors;
	int monitorCount, monitorId, retval;
	unsigned int numlockmask, winUnfocus, winFocus, 
		winInfocus, decorateWinUnfocus, decorateWinFocus, decorateWinInfocus, titleTextColor;
	Atom wmatoms[WM_COUNT], netatoms[NET_COUNT];

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
		{V_STACK_LEFT, &WindowManager::StackMode},
		{V_STACK_RIGHT,&WindowManager::StackMode},
		{H_STACK_UP,   &WindowManager::StackMode},
		{H_STACK_DOWN, &WindowManager::StackMode},
		{GRID,         &WindowManager::GridMode},
		{MONOCLE,      &WindowManager::MonocleMode},
		{FIBONACCI,    &WindowManager::FibonacciMode},
		{FLOAT,        &WindowManager::FloatMode}
	};
};

#endif
