/*
UNKNOWN WM
*/

#include <unistd.h>
#include <algorithm>
#include <unistd.h>
#include <thread>
#include <iterator>

#include "wm.h"

using namespace UW;

#define LERR SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::ERR)
#define LINFO SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::INFO)

std::unique_ptr<WindowManager> WindowManager::Create() {
	Display* display = XOpenDisplay(nullptr);
	if (display == nullptr) {
		LERR << "Failed to open display\n";
		return nullptr;
	}
	return std::unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display* display): display(display) {
}

Client* WindowManager::addWindow(Window w, Desktop* d) {
	auto cUniq = std::make_unique<Client>();
	auto c = d->addClient(std::move(cUniq), *config);
	c->win = w;
	if (config->SHOW_DECORATE) {
		c->decorateWin(display, rootWin, *config, winFocus, decorateWinFocus);
	}
	XSelectInput(display, c->win, 
		PropertyChangeMask | FocusChangeMask | (config->FOLLOW_MOUSE ? EnterWindowMask : 0));
	return c;
}

void WindowManager::changeLayout(const Argument *arg) {
	Monitor *m =  monitors[monitorId].get();
	Desktop *d = m->desktops[monitors[monitorId]->desktopCurId].get();
	auto ls = config->layouts;
	if (config->desktopLayouts.find(monitors[monitorId]->desktopCurId) != config->desktopLayouts.end()) {
		ls = config->desktopLayouts[monitors[monitorId]->desktopCurId];
	}
	d->layoutId = ((d->layoutId + arg->i) < 0 ? ls.size()-1 : (d->layoutId + arg->i)) % ls.size();
	Argument _arg(ls[d->layoutId]);
	switchMode(&_arg);
}

void WindowManager::buttonPress(XEvent *e) {
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;

	bool w = winToClient(e->xbutton.window, &c, &d, &m);
	int cm = 0; 
	while(m != monitors[cm].get() && cm < monitorCount) {
		cm++;
	}
	if (w && config->CLICK_TO_FOCUS && e->xbutton.button == config->FOCUS_BUTTON && 
		(c != d->getCur()|| cm != monitorId)) {
		if (cm != monitorId) {
			Argument _arg;
			_arg.i = cm;
			changeMonitor(&_arg);
		}
		focus(c, d, m);
	}
	for (unsigned int i = 0; i < config->buttons.size(); i++) {
		if (cleanMask(config->buttons[i].mask) == cleanMask(e->xbutton.state) &&
			config->buttons[i].func != "" && config->buttons[i].button == e->xbutton.button) {
			if (w && cm != monitorId) {
				Argument _arg;
				_arg.i = cm;
				changeMonitor(&_arg);
			}
			if (w && c != d->getCur()) {
				focus(c, d, m);
			}
			runFunc("button", config->buttons[i].func, &(config->buttons[i].arg));
		}
	}
	if (config->SHOW_DECORATE && config->USE_TITLE_BUTTON_ACTIONS) {
		buttonPressTitle(e);
	}
}

void WindowManager::buttonPressTitle(XEvent *e) {
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;

	bool w = winToClient(e->xbutton.subwindow, &c, &d, &m, true);
			
	if (w && config->USE_TITLE_BUTTON_ACTIONS) {
		int cm = 0; 
		while(m != monitors[cm].get() && cm < monitorCount) {
			cm++;
		}
		if (cm != monitorId) {
			Argument _arg;
			_arg.i = cm;
			changeMonitor(&_arg);
		}
		focus(c, d, m);

		Argument arg(0);
		if (e->xbutton.button == Button1) {
			toggleFullscreenClient(&arg);
		} else if (e->xbutton.button == Button3) {
			killClient(&arg);
		} else if (e->xbutton.button == Button2) {
			hideCurClient(&arg);
		}
	}
}

void WindowManager::changeDesktop(const Argument *arg) {
	Monitor *m = monitors[monitorId].get();

	if (arg->i == m->desktopCurId || arg->i < 0 || arg->i >= config->DESKTOPS) {
		return;
	}

	Desktop *d = m->desktops[(m->desktopPrevId = m->desktopCurId)].get();
	Desktop *n = m->desktops[(m->desktopCurId = arg->i)].get();
	
	n->mapAll(*config, display);

	XSetWindowAttributes attr1;
	attr1.do_not_propagate_mask = SubstructureNotifyMask;
	XChangeWindowAttributes(display, rootWin, CWEventMask, &attr1);

	d->unmapAll(*config, display);

	XSetWindowAttributes attr2;
	attr2.event_mask = rootMask();
	XChangeWindowAttributes(display, rootWin, CWEventMask, &attr2);

	if (n->getCur() != nullptr) { 
		tile(n, m);
		focus(n->getCur(), n, m);
	}
	
	desktopInfo();
}

void WindowManager::changeMonitor(const Argument *arg) {
	if (arg->i == monitorId || arg->i < 0 || arg->i >= monitorCount) {
		return;
	}
	Monitor *m = monitors[monitorId].get();
	monitorId = arg->i;
	Monitor *n = monitors[monitorId].get();
	Client *mClient = m->desktops[m->desktopCurId]->getCur();
	Client *nClient = n->desktops[n->desktopCurId]->getCur();
	focus(mClient, m->desktops[m->desktopCurId].get(), m);
	focus(nClient, n->desktops[n->desktopCurId].get(), n);
	desktopInfo();
}

void WindowManager::focus(Client *c, Desktop *d, Monitor *m) {
	if (!d->getHead() || !c) {
		XDeleteProperty(display, rootWin, netatoms[NET_ACTIVE]);
		d->curClientId = -1;
		d->clients.clear();
		return;
	} else if (d->getCur() != c) {
		d->prevClientId = d->getCur() ? d->getCur()->id : -1; 
		d->curClientId = c->id;
	}
	int n = d->clients.size(), fl = 0, ft = 0;
	for (auto& _c : d->clients) {
		if (isFloatOrFullscreen(_c.get())) {
			fl++;
			if (!_c->isFull) {
				ft++;
			}
		}
	}
	

	int scale = config->SHOW_DECORATE ? 2 : 1;
	Window w[n*scale];
	auto _id = (d->getCur()->isFloat || d->getCur()->isTrans) ? 0 : ft;
	w[_id*scale] = d->getCur()->win;
	if (config->SHOW_DECORATE) {
		w[_id*scale+1] = d->getCur()->decorate;
	}
	
	fl += !isFloatOrFullscreen(d->getPrev()) ? 1 : 0;

	for (auto& _c : d->clients) {
		auto win = config->SHOW_DECORATE && _c->isDecorated ? _c->decorate : _c->win;
		if (config->SHOW_DECORATE) {
			XSetWindowBackground(display, win, 
				(_c.get() != d->getCur()) ? decorateWinUnfocus : 
					(m == monitors[monitorId].get()) ? decorateWinFocus : decorateWinInfocus);
			XClearWindow(display, win);

			XftColor msgcolor;
			XftFont* msgfont;
			int screen = DefaultScreen(display);
			msgfont = XftFontOpenName(display, screen, config->FONT.c_str());
			XftColorAllocName(display, DefaultVisual(display, screen), 
				DefaultColormap(display, screen), config->TITLE_TEXT_COLOR.c_str(), &msgcolor);

			XftDraw *draw = XftDrawCreate(display, win,
				DefaultVisual(display, screen), DefaultColormap(display, screen));
			
			if (config->TITLE_POSITION == TITLE_UP) {
				XftDrawString8(draw, &msgcolor, msgfont, config->TITLE_DX, config->TITLE_DY, 
					(XftChar8*)(_c->title.c_str()), 
					_c->title.size());
			}
			if (config->TITLE_POSITION == TITLE_DOWN) {
				XWindowAttributes wa;
				if (XGetWindowAttributes(display, win, &wa)) {
					XftDrawString8(draw, &msgcolor, msgfont, 
						config->TITLE_DX, 
						wa.height-config->TITLE_DY, 
						(XftChar8*)(_c->title.c_str()), 
						_c->title.size());
				}
				
			}
			if (config->TITLE_POSITION == TITLE_LEFT) {
				XWindowAttributes wa;
				if (XGetWindowAttributes(display, win, &wa)) {
					for (auto i = 0; i < _c->title.size(); ++i) {
						std::string s(1, _c->title[i]);
						XftDrawString8(draw, &msgcolor, msgfont, 
							config->TITLE_DX, 
							config->TITLE_DY + i*msgfont->height, 
							(XftChar8*)s.c_str(), 1);
					}
				}
				
			}
			if (config->TITLE_POSITION == TITLE_RIGHT) {
				XWindowAttributes wa;
				if (XGetWindowAttributes(display, win, &wa)) {
					for (auto i = 0; i < _c->title.size(); ++i) {
						std::string s(1, _c->title[i]);
						XftDrawString8(draw, &msgcolor, msgfont, 
							wa.width-config->TITLE_DX, 
							config->TITLE_DY + i*msgfont->height, 
							(XftChar8*)s.c_str(), 1);
					}
				}
				
			}
			XFlush(display);
			XftDrawDestroy(draw);


			/*OLD VARIANT
			GC gr; 
			XGCValues gr_values;
			gr_values.foreground = titleTextColor;
			gr = XCreateGC(display,win, 
				GCForeground, 
				&gr_values);
			//XSetFont (display, gr, fontInfo->fid);
			if (config->TITLE_POSITION == TITLE_UP) {
				XDrawString(display, win, gr, 
					        config->TITLE_DX, 
					        config->TITLE_DY, 
					        (*_c)->title.c_str(), 
					        (*_c)->title.size());
			}
			if (config->TITLE_POSITION == TITLE_DOWN) {
				XWindowAttributes wa;
				if (XGetWindowAttributes(display, win, &wa)) {
					XDrawString(display, win, gr, 
						config->TITLE_DX, 
						wa.height-config->TITLE_DY, 
						(*_c)->title.c_str(), 
						(*_c)->title.size());
				}
				
			}
			if (config->TITLE_POSITION == TITLE_LEFT) {
				XWindowAttributes wa;
				if (XGetWindowAttributes(display, win, &wa)) {
					for (auto i = 0; i < (*_c)->title.size(); ++i) {
						std::string s(1, (*_c)->title[i]);
						XDrawString(display, win, gr, 
							config->TITLE_DX, 
							config->TITLE_DY + i*12, 
							s.c_str(), 
							1);
					}
				}
				
			}
			if (config->TITLE_POSITION == TITLE_RIGHT) {
				XWindowAttributes wa;
				if (XGetWindowAttributes(display, win, &wa)) {
					for (auto i = 0; i < (*_c)->title.size(); ++i) {
						std::string s(1, (*_c)->title[i]);
						XDrawString(display, win, gr, 
							wa.width-config->TITLE_DX, 
							config->TITLE_DY + i*12, 
							s.c_str(), 
							1);
					}
				}
				
			}
			XFreeGC(display, gr);*/
			//XUnloadFont(display, fontInfo->fid);
		}
		XSetWindowBorder(display, win, (_c.get() != d->getCur()) ? winUnfocus : (m == monitors[monitorId].get()) ? winFocus : winInfocus);

		XSetWindowBorderWidth(display, win, _c->isFull || (!isFloatOrFullscreen(_c.get()) &&
			(d->mode == MONOCLE || !d->clients.size() > 1)) ? 0 : config->BORDER_WIDTH);

		if (_c.get() != d->getCur()) {
			auto id = (_c->isFull ? --fl:isFloatOrFullscreen(_c.get()) ? --ft:--n) * scale;
			w[id] = _c->win;
			if (config->SHOW_DECORATE) {
				w[id+1] = _c->decorate;
			}
		}
		if (config->CLICK_TO_FOCUS || _c.get() == d->getCur()) {
			grabButtons(_c.get());
		}
	}
	
	XRestackWindows(display, w, Length(w));

	XSetInputFocus(display, d->getCur()->win, RevertToPointerRoot, CurrentTime);
	XChangeProperty(display, rootWin, netatoms[NET_ACTIVE], XA_WINDOW, 32,
		            PropModeReplace, (unsigned char*)&d->getCur()->win, 1);
	XSync(display, false);
}

void WindowManager::cleanup() {
	Window rootReturn, parentReturn, *children;
	unsigned int childrenCount;

	XUngrabKey(display, AnyKey, AnyModifier, rootWin);
	XQueryTree(display, rootWin, &rootReturn, &parentReturn, &children, &childrenCount);
	for (unsigned int i = 0; i < childrenCount; i++) {
		deleteWindow(children[i]);
	}
	if (children) {
		XFree(children);
	}
	XSync(display, false);

	monitors.clear();
	config.reset();
}

void WindowManager::clientToDesktop(const Argument *arg) {
	Monitor *m = monitors[monitorId].get();
	Desktop *d = m->desktops[m->desktopCurId].get();
	Desktop *n = nullptr;
	if (arg->i == m->desktopCurId || arg->i < 0 || arg->i >= config->DESKTOPS || !d->getCur()) {
		return;
	}

	auto cUniq = std::move(d->clients[d->curClientId]);

	Client *c = cUniq.get();
	c->isUnmap = true;
	n = m->desktops[arg->i].get();
	d->clients.erase(d->clients.begin()+c->id);
	int i = 0;
	for (auto& _c : d->clients) {
		_c->id = i;
		i++;
	}
	
	XSetWindowAttributes _attr1;
	_attr1.do_not_propagate_mask = SubstructureNotifyMask;
	XChangeWindowAttributes(display, rootWin, CWEventMask, &_attr1);
	if (XUnmapWindow(display, c->win)) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			XUnmapWindow(display, c->decorate);
		}
		focus(d->getHead(), d, m);
	}
	XSetWindowAttributes _attr2;
	_attr2.event_mask = rootMask();
	XChangeWindowAttributes(display, rootWin, CWEventMask, &_attr2);
	if (!(c->isFloat || c->isTrans) || (d->getHead() && d->clients.size() > 1)) {
		tile(d, m);
	}
	c->id = n->clients.size();
	n->clients.push_back(std::move(cUniq));
	focus(c, n, m);
	if (config->FOLLOW_WINDOW) {
		changeDesktop(arg);
	}
	desktopInfo();
}

void WindowManager::clientToMonitor(const Argument *arg) {
	Monitor *cm = monitors[monitorId].get();
	Monitor *nm = nullptr;
	Desktop *cd = cm->desktops[cm->desktopCurId].get();
	Desktop *nd = nullptr;
	if (arg->i == monitorId || arg->i < 0 || arg->i >= monitorCount || 
		!cd->getCur()) {
		return;
	}
	nm = monitors[arg->i].get();
	nd = nm->desktops[nm->desktopCurId].get();

	auto cUniq = std::move(cd->clients[cd->curClientId]);
	Client *c = cUniq.get();
	cd->clients.erase(cd->clients.begin() + cd->getCur()->id);
	int i = 0;
	for (auto& _c : cd->clients) {
		_c->id = i;
		i++;
	}

	focus(cd->getHead(), cd, cm);
	if (!(c->isFloat || c->isTrans) || (cd->getHead() && cd->clients.size() > 1)) {
		tile(cd, cm);
	}
	if (isFloatOrFullscreen(c)) {
		c->isFloat = c->isFull = false;
	}
	nd->clients.push_back(std::move(cUniq));
	c->id = nd->clients.size()-1;
	focus(c, nd, nm);
	tile(nd, nm);
	if (config->FOLLOW_MONITOR) {
		changeMonitor(arg);
	}
	desktopInfo();
}

void WindowManager::eventFromClient(XEvent* e) {
	if (e->xclient.data.l[0] == 0) {
		Argument a(e->xclient.data.l[1]);
		changeDesktop(&a);
		return;
	}
    if (e->xclient.data.l[0] == 1) {
		Argument a(e->xclient.data.l[1]);
		changeMonitor(&a);
		return;
	}
    if (e->xclient.data.l[0] == 2) {
		Argument a(e->xclient.data.l[1]);
		switchMode(&a);
		return;
	}
    if (e->xclient.data.l[0] == 3) {
		Argument a(0);
		quit(&a);
		return;
	}
    if (e->xclient.data.l[0] == 4) {
		Argument a(e->xclient.data.l[1]);
		togglePanel(&a);
		return;
	}
    if (e->xclient.data.l[0] == 5) {
		Argument a(e->xclient.data.l[1]);
		nextWin(&a);
		return;
	}
    if (e->xclient.data.l[0] == 6) {
		Argument a(e->xclient.data.l[1]);
		prevWin(&a);
		return;
	}
    if (e->xclient.data.l[0] == 7) {
		Argument a(e->xclient.data.l[1]);
		nextDesktop(&a);
		return;
	}
    if (e->xclient.data.l[0] == 8) {
		Argument a(e->xclient.data.l[1]);
		prevDesktop(&a);
		return;
	}
    if (e->xclient.data.l[0] == 9) {
		Argument a(e->xclient.data.l[1]);
		toggleFullscreenClient(&a);
		return;
	}
    if (e->xclient.data.l[0] == 10) {
		Argument a(e->xclient.data.l[1]);
		toggleFloatClient(&a);
		return;
	}
    if (e->xclient.data.l[0] == 11) {
		Argument a(1);
		changeLayout(&a);
		return;
	}
    if (e->xclient.data.l[0] == 12) {
		Argument a(-1);
		changeLayout(&a);
		return;
	}
    if (e->xclient.data.l[0] == 13) {
		Argument a(e->xclient.data.l[1]);
		restart(&a);
		return;
	}
    if (e->xclient.data.l[0] == 14) {
		Argument a(e->xclient.data.l[1]);
		restartMonitors(&a);
		return;
	}

}

void WindowManager::clientMessage(XEvent* e) {
	if (e->xclient.message_type == unknowwm_client_event) {
		eventFromClient(e);
		return;
	}
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;
	if (!winToClient(e->xclient.window, &c, &d, &m)) {
		return;
	}
	if (e->xclient.message_type        == netatoms[NET_WM_STATE] && (
		(unsigned)e->xclient.data.l[1] == netatoms[NET_FULLSCREEN] ||
		(unsigned)e->xclient.data.l[2] == netatoms[NET_FULLSCREEN])) {
		fullscreenMode(c, d, m, (e->xclient.data.l[0] == 1 || (e->xclient.data.l[0] == 2 && !c->isFull)));
		if (!(c->isFloat || c->isTrans) || !d->clients.size() > 1) {
			tile(d, m);
		}
	} else if (e->xclient.message_type == netatoms[NET_ACTIVE]) {
		focus(c, d, m);
	}
}

void WindowManager::configureRequest(XEvent *e) {
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc = { ev->x, ev->y,  ev->width, ev->height, ev->border_width, ev->above, ev->detail };
	if (XConfigureWindow(display, ev->window, ev->value_mask, &wc)) {
		XSync(display, false);
	}
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;
	if (winToClient(ev->window, &c, &d, &m)) {
		tile(d, m);
	}
}

void WindowManager::deleteWindow(Window w) {
	XEvent ev = { .type = ClientMessage };
	ev.xclient.window = w;
	ev.xclient.format = 32;
	ev.xclient.message_type = wmatoms[WM_PROTOCOLS];
	ev.xclient.data.l[0]    = wmatoms[WM_DELETE_WINDOW];
	ev.xclient.data.l[1]    = CurrentTime;
	XSendEvent(display, w, false, NoEventMask, &ev);
}

void WindowManager::desktopInfo() {
	if (config->SEND_STATUS_SCRIPT.empty()) {
		return;
	}
	Monitor *m = nullptr;
	Client *c = nullptr;
	bool urgent = false;

	std::string info = "{\n";
	info += "\"monitors\": [\n";
	for (int cm = 0; cm < monitorCount; cm++) {
		info += "{\n";
		info += (monitorId == cm ? "\"cur\": true,\n" : "\"cur\": false,\n");
		info += "\"desktops\": [\n";
		for (int cd = 0; cd < config->DESKTOPS; cd++) {
			m = monitors[cm].get();
			info += "{\n";
			info += (m->desktopCurId == cd ? "\"cur\": true,\n" : "\"cur\": false,\n");
			info += "\"mode\":";
			info += std::to_string(m->desktops[cd]->mode) + ",\n";
			info += "\"clients\": [\n";
			auto cc = 0;
			for (auto& _c : m->desktops[cd]->clients) {
				info += "{\n";
				info += "\"title\": \"" + _c->title + "\",";
				if (m->desktops[cd]->curClientId == _c->id) {
					info += "\"cur\": true";
				} else {
					info += "\"cur\": false";
				}
				info += "}";
				if (cc != m->desktops[cd]->clients.size()-1) {
					info += ",\n";
				} else {
					info += "\n";
				}
				cc++;
			}
			info += "]\n";
			info += "}\n";
			if (cd != config->DESKTOPS-1) {
				info += ",\n";
			} else {
				info += "\n";
			}
		}
		info += "]";
		info += "}";
		if (cm != monitorCount-1) {
			info += ",\n";
		} else {
			info += "\n";
		}
	}
	info += "]\n";
	info += "}\n";
	//LINFO << info;

	std::istringstream iss(config->SEND_STATUS_SCRIPT);
	std::vector<std::string> cmd((std::istream_iterator<std::string>(iss)),
		std::istream_iterator<std::string>());
	cmd.push_back(info);
	Argument a(cmd);
	runCmd(&a);
}

void WindowManager::destroyNotify(XEvent *e) {
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;
	if (winToClient(e->xdestroywindow.window, &c, &d, &m)) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			c->decorationsDestroy(display);
		}
		removeClient(c, d, m);
	}
}

void WindowManager::enterNotify(XEvent *e) {
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;
	Client *p = nullptr;

	if (!config->FOLLOW_MOUSE || 
		(e->xcrossing.mode != NotifyNormal && 
		e->xcrossing.detail == NotifyInferior) || 
		!winToClient(e->xcrossing.window, &c, &d, &m) || 
		e->xcrossing.window == d->getCur()->win) {
		return;
	}
	if (m != monitors[monitorId].get()) {
		for (int cm = 0; cm < monitorCount; cm++) {
			if (m == monitors[cm].get()) {
				Argument _arg;
				_arg.i = cm;
				changeMonitor(&_arg);
			}
		}
	}
	if ((p = d->getPrev())) {
		XSetWindowAttributes attr;
		attr.do_not_propagate_mask = EnterWindowMask;
		XChangeWindowAttributes(display, p->win, CWEventMask, &attr);
	}
	focus(c, d, m);
	if (p) {
		XSetWindowAttributes attr;
		attr.event_mask = EnterWindowMask;
		XChangeWindowAttributes(display, p->win, CWEventMask, &attr);
	}
}

void WindowManager::focusIn(XEvent *e) {
	Monitor *m = monitors[monitorId].get();
	Desktop *d = m->desktops[m->desktopCurId].get();
	if (d->getCur() && d->getCur()->win != e->xfocus.window) {
		focus(d->getCur(), d, m);
	}
}

void WindowManager::focusUrGent() {
	Monitor *m = monitors[monitorId].get();
	Client *c = nullptr;
	int d = -1;
	auto _c = m->desktops[m->desktopCurId]->clients.begin();
	for (_c = m->desktops[m->desktopCurId]->clients.begin(); 
		_c != m->desktops[m->desktopCurId]->clients.end() && !(*_c)->isUrgn; _c++);
	c = _c->get();
	while (!c && d < config->DESKTOPS-1) {
		for (auto _c = m->desktops[++d]->clients.begin(); 
			_c != m->desktops[m->desktopCurId]->clients.end() && !(*_c)->isUrgn; c++);
		c = _c->get();
	}
	if (c) {
		if (d != -1) {
			Argument _arg;
			_arg.i = d;
			changeDesktop(&_arg);
			focus(c, m->desktops[m->desktopCurId].get(), m); 
		}
	}
}

void WindowManager::grabButtons(Client *c) {
	Monitor *cm = monitors[monitorId].get();
	unsigned int b, m, modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };

	for (m = 0; config->CLICK_TO_FOCUS && m < Length(modifiers); m++) {
		if (c != cm->desktops[cm->desktopCurId]->getCur()) {
			XGrabButton(display, config->FOCUS_BUTTON, modifiers[m],
				c->win, false, buttonMask(), GrabModeAsync, GrabModeAsync, None, None);
		} else {
			XUngrabButton(display, config->FOCUS_BUTTON, modifiers[m], c->win);
		}
	}
	for (b = 0, m = 0; b < config->buttons.size(); b++, m = 0) {
		while (m < Length(modifiers)) {
			XGrabButton(display, config->buttons[b].button, 
				        config->buttons[b].mask|modifiers[m++], c->win,
				        false, buttonMask(), GrabModeAsync, GrabModeAsync, None, None);
		}
	}
}


void WindowManager::grabKeys() {
	KeyCode code;
	XUngrabKey(display, AnyKey, AnyModifier, rootWin);
	unsigned int k, m, modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };

	for (k = 0, m = 0; k < config->keys.size(); k++, m = 0) {
		while ((code = XKeysymToKeycode(display, config->keys[k].keysym)) && m < Length(modifiers)) {
			XGrabKey(display, code, config->keys[k].mod | modifiers[m++], rootWin, true, GrabModeAsync, GrabModeAsync);
		}
	}
}

void WindowManager::keyPress(XEvent *e) {
	KeySym keysym = XkbKeycodeToKeysym(display, e->xkey.keycode, 0, 0);
	for (unsigned int i = 0; i < config->keys.size(); i++) {
		if (keysym == config->keys[i].keysym && 
			cleanMask(config->keys[i].mod) == cleanMask(e->xkey.state)) {
			if (config->keys[i].func != "") {
				runFunc("keys", config->keys[i].func, &config->keys[i].arg);
			}
		}
	}
}

void WindowManager::runFunc(std::string type, std::string funcStr, const Argument *arg) {
	if (type == "keys") {
		(*this.*runFuncMap[funcStr])(arg);
	}
	if (type =="button") {
		if (funcStr == "MouseMotion") {
			mouseMotion(arg);
		}
	}
}

void WindowManager::killClient(const Argument *arg) {
	Monitor *m = monitors[monitorId].get();
	Desktop *d = m->desktops[m->desktopCurId].get();
	if (!d->getCur()) {
		return;
	}
	Atom *prot = nullptr;
	int n = -1;
	if (XGetWMProtocols(display, d->getCur()->win, &prot, &n)) {
		while(--n >= 0 && prot[n] != wmatoms[WM_DELETE_WINDOW]);
	}
	if (config->SHOW_DECORATE) {
		d->getCur()->decorationsDestroy(display);
	}
	if (n < 0) { 
		XKillClient(display, d->getCur()->win);
		removeClient(d->getCur(), d, m); 
	} else {
		deleteWindow(d->getCur()->win);
	}
	
	if (prot) {
		XFree(prot);
	}
}

void WindowManager::prevDesktop(const Argument *_arg) {
	Argument arg;
	arg.i = monitors[monitorId]->desktopPrevId;
	changeDesktop(&arg);
}

void WindowManager::mapRequest(XEvent *e) {
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;
	Window w = e->xmaprequest.window;
	XWindowAttributes wa = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	if (winToClient(w, &c, &d, &m) || (XGetWindowAttributes(display, w, &wa) && wa.override_redirect)) {
		return;
	}
	XClassHint ch = {0, 0};
	bool follow = false;
	bool floating = false;
	int newmon = monitorId;
	int newdsk = monitors[monitorId]->desktopCurId;

	if (XGetClassHint(display, w, &ch)) {
		for (unsigned int i = 0; i < config->rules.size(); i++) {
			if (strstr(ch.res_class, config->rules[i].appClass.c_str()) || strstr(ch.res_name, config->rules[i].appClass.c_str())) {
				if (config->rules[i].monitor >= 0 && config->rules[i].monitor < monitorCount) {
					newmon = config->rules[i].monitor;
				}
				if (config->rules[i].desktop >= 0 && config->rules[i].desktop < config->DESKTOPS) {
					newdsk = config->rules[i].desktop;
				}
				follow = config->rules[i].isFollow;
				floating = config->rules[i].isFloating;
				break;
			}
		}
	}
	
	if (ch.res_class) {
		XFree(ch.res_class);
	}
	if (ch.res_name) {
		XFree(ch.res_name);
	}
	m = monitors[newmon].get();
	d = m->desktops[newdsk].get();
	
	c = addWindow(w, d);
	if (c == nullptr) {
		return;
	}
	c->isFull = false;
	c->isTrans = XGetTransientForHint(display, c->win, &w);
	if ((c->isFloat = (floating || d->mode == FLOAT_MODE)) && !c->isTrans) {
		auto win = config->SHOW_DECORATE ? c->decorate : c->win;
		XMoveWindow(display, win, m->x + (m->w - wa.width)/2, m->y + (m->h - wa.height)/2);

		if (config->SHOW_DECORATE && c->isDecorated) {
			c->moveLocal(m->x + (m->w - wa.width)/2, 
				m->y + (m->h - wa.height)/2, *config, display);
		}
	}
	int i;
	unsigned long l;
	unsigned char *state = nullptr;
	Atom a;
	if (XGetWindowProperty(display, c->win, netatoms[NET_WM_STATE], 0L, sizeof a,
		false, XA_ATOM, &a, &i, &l, &l, &state) == Success && state) {
		fullscreenMode(c, d, m, (*(Atom *)state == netatoms[NET_FULLSCREEN]));
	}
	if (state) {
		XFree(state);
	}

	XTextProperty name;
	XGetTextProperty(display, c->win, &name, XA_WM_NAME);
	if (name.nitems && name.encoding == XA_STRING) {
		c->title = (char*)name.value;
	}
	if (name.value) {
		XFree(name.value);
	}

	if (m->desktopCurId == newdsk) {
		
		if (/*!IsFloatOrFullscreen(c)*/!c->isFloat) {
			tile(d, m);
		}
		auto _win = config->SHOW_DECORATE ? c->decorate : c->win;
		XMapWindow(display, _win);
		if (config->SHOW_DECORATE && c->isDecorated) {
			XMapWindow(display, c->win);
		}
		
	}
	if (follow) {
		Argument arg1;
		arg1.i = newmon;
		Argument arg2;
		arg2.i = newdsk;
		changeMonitor(&arg1);
		changeDesktop(&arg2);
	}
	//if (!c->isIgnore)
	focus(c, d, m);
	
	desktopInfo();
}

void WindowManager::mouseMotion(const Argument *Argument) {
	Monitor *m = monitors[monitorId].get();
	Desktop *d = m->desktops[m->desktopCurId].get();
	XWindowAttributes wa;
	XEvent ev;
	if (!d->getCur() || !XGetWindowAttributes(display, d->getCur()->win, &wa)) {
		return;
	}
	if (!d->getCur()->isFloat && !d->getCur()->isTrans && !config->AUTOFLOATING) {
		return;
	}
	if (Argument->i == RESIZE) {
		XWarpPointer(display, d->getCur()->win, d->getCur()->win, 0, 0, 0, 0, --wa.width, --wa.height);
	}
	int rx, ry, c, xw, yh; 
	unsigned int v; 
	Window w;
	if (!XQueryPointer(display, rootWin, &w, &w, &rx, &ry, &c, &c, &v) || w != d->getCur()->win) {
		return;
	}
	if (XGrabPointer(display, rootWin, false, buttonMask()|PointerMotionMask, GrabModeAsync,
		GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
			return;
	}
	if (!d->getCur()->isFloat && !d->getCur()->isTrans) {
		d->getCur()->isFloat = true;
		tile(d, m);
		focus(d->getCur(), d, m);
	}
	if (config->SHOW_DECORATE) {
		XRaiseWindow(display, d->getCur()->decorate);
	}
	XRaiseWindow(display, d->getCur()->win);
	do {
		XMaskEvent(display, ButtonPressMask|ButtonReleaseMask |
			PointerMotionMask | SubstructureRedirectMask, &ev);
		if (ev.type == MotionNotify) {
			xw = (Argument->i == MOVE ? wa.x:wa.width)  + ev.xmotion.x - rx;
			yh = (Argument->i == MOVE ? wa.y:wa.height) + ev.xmotion.y - ry;
			if (Argument->i == RESIZE) {
				auto win = config->SHOW_DECORATE ? d->getCur()->decorate : d->getCur()->win;
				if (config->SHOW_DECORATE && d->getCur()->isDecorated) {
					d->getCur()->resizeLocal((xw > config->MINWSZ ? xw:wa.width), 
						(yh > config->MINWSZ ? yh:wa.height), *config, display);
				}
				XResizeWindow(display, win,
					xw > config->MINWSZ ? xw:wa.width, yh > config->MINWSZ ? yh:wa.height);
			} else if (Argument->i == MOVE) {
				auto win = config->SHOW_DECORATE ? d->getCur()->decorate : d->getCur()->win;
				if (config->SHOW_DECORATE && d->getCur()->isDecorated) {
					d->getCur()->moveLocal(xw, yh, *config, display);
				}
				XMoveWindow(display, win, xw, yh);
				
			}
		} else if (ev.type == ConfigureRequest || ev.type == MapRequest) {
			(*this.*events[ev.type])(&ev);
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(display, CurrentTime);
}

void WindowManager::moveDown(const Argument *arg) {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId].get();
	if (!d->getCur() || d->clients.size() <= 1) {
		return;
	}
	
	auto _id1 = d->getCur()->id;
	auto _id2 = (_id1 + 1 < d->clients.size()) ? _id1 + 1 : 0;
	
	d->clients[_id1]->id = _id2;
	d->clients[_id2]->id = _id1;

	auto elem = std::move(d->clients[_id1]);
	d->clients[_id1] = std::move(d->clients[_id2]);
	d->clients[_id2] = std::move(elem);
	
	d->curClientId = _id2;
	d->prevClientId = _id2 > 0 ? _id2-1 : d->clients.size()-1;

	if (!d->getCur()->isFloat && !d->getCur()->isTrans) {
		tile(d, monitors[monitorId].get());
	}
}

void WindowManager::moveUp(const Argument *arg) {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId].get();
	if (!d->getCur() || d->clients.size() <= 1) {
		return;
	}
		
	auto _id1 = d->getCur()->id;
	auto _id2 = (_id1 - 1 >= 0) ? _id1 - 1 : d->clients.size()-1;
	
	d->clients[_id1]->id = _id2;
	d->clients[_id2]->id = _id1;

	auto elem = std::move(d->clients[_id1]);
	d->clients[_id1] = std::move(d->clients[_id2]);
	d->clients[_id2] = std::move(elem);

	d->curClientId = _id2;
	d->prevClientId = _id2 > 0 ? _id2-1 : d->clients.size()-1;
	
	if (!d->getCur()->isFloat && !d->getCur()->isTrans) {
		tile(d, monitors[monitorId].get());
	}
}

void WindowManager::toggleFloatClient(const Argument *arg) {
	Monitor *m = monitors[monitorId].get();
	Desktop *d = m->desktops[m->desktopCurId].get();
	XWindowAttributes wa;
	if (!d->getCur()) {
		return;
	}
	auto win = config->SHOW_DECORATE ? d->getCur()->decorate : d->getCur()->win;
	if (!XGetWindowAttributes(display, win, &wa)) {
		return;
	}
	d->getCur()->isFloat = !d->getCur()->isFloat;
	tile(d, m);
	focus(d->getCur(), d, m);
	if (d->getCur()->isFloat) {
		XRaiseWindow(display, win);
		//m->w/4, m->h/4, m->w/2, m->h/2
		XMoveResizeWindow(display, win, (m->w-wa.width)/2, (m->h-wa.height)/2,
			              wa.width, wa.height);
		if (config->SHOW_DECORATE && d->getCur()->isDecorated) {
			XRaiseWindow(display, d->getCur()->win);
			d->getCur()->moveResizeLocal((m->w-wa.width)/2, (m->h-wa.height)/2,
			                wa.width, wa.height, *config, display);
		}
	}
}

void WindowManager::toggleFullscreenClient(const Argument *arg) {
	Monitor *m = monitors[monitorId].get();
	Desktop *d = m->desktops[m->desktopCurId].get();
	if (!d->getCur()) {
		return;
	}
	fullscreenMode(d->getCur(), d, m, !d->getCur()->isFull);
}

void WindowManager::moveResize(const Argument *Argument) {
	Monitor *m = monitors[monitorId].get();
	Desktop *d = m->desktops[m->desktopCurId].get();
	XWindowAttributes wa;
	if (!d->getCur() || !XGetWindowAttributes(display, d->getCur()->win, &wa)) {
		return;
	}
	if (!d->getCur()->isFloat && !d->getCur()->isTrans && !config->AUTOFLOATING) {
		return;
	}
	if (!d->getCur()->isFloat && !d->getCur()->isTrans) {
		d->getCur()->isFloat = true;
		tile(d, m);
		focus(d->getCur(), d, m);
	}

	auto win = config->SHOW_DECORATE ? d->getCur()->decorate : d->getCur()->win;
	XRaiseWindow(display, win);
	XMoveResizeWindow(display, win, wa.x + Argument->intArr[0], wa.y + Argument->intArr[1],
		wa.width + Argument->intArr[2], wa.height + Argument->intArr[3]);
	if (config->SHOW_DECORATE && d->getCur()->isDecorated) {
		XRaiseWindow(display, d->getCur()->win);
		d->getCur()->moveResizeLocal(wa.x + Argument->intArr[0], wa.y + Argument->intArr[1],
			wa.width + Argument->intArr[2], wa.height + Argument->intArr[3], *config, display);
	}
}

void WindowManager::focusWinById(const Argument *arg) {//arg is id
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId].get();
	if (d->getCur() && d->clients.size() > arg->i) {
		auto c = d->clients[arg->i].get();
		if (c->isHide) {
			hideClient(c, monitors[monitorId].get());
		}
		focus(c, d, monitors[monitorId].get());
	}
}

void WindowManager::nextWin(const Argument *arg) {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId].get();
	if (d->getCur() && d->clients.size() > 1) {
		auto c = (d->getCur()->id + 1 < d->clients.size()) ? d->clients[d->getCur()->id + 1].get() : d->clients[0].get();
		if (c->isHide) {
			hideClient(c, monitors[monitorId].get());
		}
		focus(c, d, monitors[monitorId].get());
	}
}

void WindowManager::propertyNotify(XEvent *e) {
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;

	if (!winToClient(e->xproperty.window, &c, &d, &m)) {
		return;
	}

	if (e->xproperty.atom == XA_WM_HINTS) {
		XWMHints *wmh = XGetWMHints(display, c->win);
		Desktop *cd = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId].get();
		c->isUrgn = (c != cd->getCur() && wmh && (wmh->flags & XUrgencyHint));
		if (wmh) {
			XFree(wmh);
		}
	} else if (e->xproperty.atom == XA_WM_NAME) {
		XTextProperty name;
		XGetTextProperty(display, c->win, &name, XA_WM_NAME);
		if (name.nitems && name.encoding == XA_STRING) {
			c->title = (char *)name.value;
			focus(c, d, m);
		}
		if (name.value) {
			XFree(name.value);
		}
	}
	
	//desktopInfo();
}

void WindowManager::Sigchld(int sig) {
	if (signal(SIGCHLD, Sigchld) != SIG_ERR) {
		while(0 < waitpid(-1, NULL, WNOHANG));
	} else {
		LERR << "Cannot install SIGCHLD handler";
		//exit(-1);
	}
}

void WindowManager::restart(const Argument *arg) {
	for (auto i = 0; i < monitors.size(); ++i) {
		for (auto d = 0; d < monitors[i]->desktops.size(); ++d) {
			for (auto c = 0; c < monitors[i]->desktops[d]->clients.size(); ++c) {
				killClient(nullptr);
			}
		}
	}

	monitors.clear();
	std::string path = config->CONFIG_PATH;
	config = std::make_unique<Config>(path);
	init();
}

void WindowManager::restartMonitors(const Argument *arg) {
	XRRScreenResources *screenList = XRRGetScreenResources(display, rootWin);
	if (!screenList) {
		LERR << "Xrandr is not active\n";
		exit(-1);
	}

	const int _monitorCount = screenList->ncrtc;
	XRRCrtcInfo *mscreen = nullptr;
	std::vector<Rect> rects;
	for (int m = 0; m < _monitorCount; m++) {
		mscreen = XRRGetCrtcInfo(display, screenList, 
			screenList->crtcs[m]);
		if (!mscreen->mode) {
			continue;
		}
		rects.push_back(Rect{mscreen->x, mscreen->y, mscreen->width, mscreen->height});
		XRRFreeCrtcInfo(mscreen);
	}

	if (rects.size() == monitors.size()) {
		int i = 0;
		for (auto& m : monitors) {
			m->x = rects[i].x;
			m->y = rects[i].y;
			m->w = rects[i].w;
			m->h = rects[i].h;
			i++;
		}
	} else if (rects.size() < monitors.size()) {
		int i = 0;
		for (auto& r : rects) {
			monitors[i]->x = r.x;
			monitors[i]->y = r.y;
			monitors[i]->w = r.w;
			monitors[i]->h = r.h;
			i++;
		}
		for (auto j = rects.size(); j < monitors.size(); j++) {
			for (auto d = 0; d < monitors[j]->desktops.size(); ++d) {
				for (auto c = 0; c < monitors[j]->desktops[d]->clients.size(); ++c) {
					monitors[0]->desktops[0]->clients.push_back(std::move(monitors[j]->desktops[d]->clients[c]));
				}
			}
			monitors.erase(monitors.begin() + j);
		}
	} else {
		int i = 0;
		for (auto& m : monitors) {
			m->x = rects[i].x;
			m->y = rects[i].y;
			m->w = rects[i].w;
			m->h = rects[i].h;
			i++;
		}
		for (auto j = monitors.size(); j < rects.size(); j++) {
			auto monitor = std::make_unique<Monitor>(config->DESKTOPS);
			monitor->x = rects[j].x;
			monitor->y = rects[j].y;
			monitor->w = rects[j].w;
			monitor->h = rects[j].h;
			monitors.push_back(std::move(monitor));
			for (unsigned int d = 0; d < config->DESKTOPS; d++) {
				auto desk = std::make_unique<Desktop>();
				desk->nm = config->NMASTER;
				auto mode = config->initLayout[d] != -1 ? config->initLayout[d] : config->DEFAULT_MODE;
				desk->mode = mode;
				desk->isBar = config->SHOW_PANEL;
				monitors[monitors.size()-1]->desktops[d] = std::move(desk);
			}
		}
	}

	XRRFreeScreenResources(screenList);
	
}

void WindowManager::init() {

	WindowManager::Sigchld(0);
	const int screen = DefaultScreen(display);
	rootWin = RootWindow(display, screen);

	/* initialize monitors and desktops */
	/*XineramaScreenInfo *info = XineramaQueryScreens(display, &monitorCount);
	if (!monitorCount || !info){
		Logger::Err("Xinerama is not active");
		exit(-1);
	}*/
	
	for (auto i = 0; i < config->autostart.size(); ++i) {
		runCmd(&config->autostart[i]);
	}
	///
	/*
	auto monitor = std::make_unique<Monitor>(config->DESKTOPS);
	monitor->x = 0;
	monitor->y = 0;
	monitor->w = 800;
	monitor->h = 600;
	monitors.push_back(std::move(monitor));
	for (unsigned int d = 0; d < config->DESKTOPS; d++) {
		auto desk = std::make_unique<Desktop>();
		desk->nm = config->NMASTER;
		auto mode = config->initLayout[d] != -1 ? config->initLayout[d] : config->DEFAULT_MODE;
		desk->mode = mode;
		desk->isBar = config->SHOW_PANEL;
		monitors[0]->desktops[d] = std::move(desk);
	}
	*/
	///
	XRRScreenResources* screenList = XRRGetScreenResources(display, rootWin);
	if (!screenList) {
		LERR << "Xrandr is not active";
		exit(-1);
	}

	const int _monitorCount = screenList->ncrtc;
	//monitors.resize(monitorCount);
	XRRCrtcInfo *mscreen = nullptr;
	for (int m = 0; m < _monitorCount; m++) {
		
		mscreen = XRRGetCrtcInfo(display, screenList, 
			screenList->crtcs[m]);
		if (!mscreen->mode) {
			continue;
		}
		
		auto monitor = std::make_unique<Monitor>(config->DESKTOPS);
		monitor->x = mscreen->x;
		monitor->y = mscreen->y;
		monitor->w = mscreen->width;
		monitor->h = mscreen->height;
		XRRFreeCrtcInfo(mscreen);
		std::string monStr="monitor: " + std::to_string(monitor->x)+" "+
			 std::to_string(monitor->y)+" "+
			  std::to_string(monitor->w)+" "+
			   std::to_string(monitor->h)+"\n";
		LINFO << monStr;
		monitors.push_back(std::move(monitor));
		for (unsigned int d = 0; d < config->DESKTOPS; d++) {
			auto desk = std::make_unique<Desktop>();
			desk->nm = config->NMASTER;
			auto mode = config->initLayout[d] != -1 ? config->initLayout[d] : config->DEFAULT_MODE;
			desk->mode = mode;
			desk->isBar = config->SHOW_PANEL;
			monitors[m]->desktops[d] = std::move(desk);
		}
	}
	XRRFreeScreenResources(screenList);
	
	//XFree(info);
	monitorId = 0;
	monitorCount = monitors.size();

	winFocus = getColor(config->FOCUS_COLOR, screen, display);
	winUnfocus = getColor(config->UNFOCUS_COLOR, screen, display);
	winInfocus = getColor(config->INFOCUS_COLOR, screen, display);

	decorateWinFocus = getColor(config->DECORATE_FOCUS_COLOR, screen, display);
	decorateWinUnfocus = getColor(config->DECORATE_UNFOCUS_COLOR, screen, display);
	decorateWinInfocus = getColor(config->DECORATE_INFOCUS_COLOR, screen, display);

	titleTextColor = getColor(config->TITLE_TEXT_COLOR, screen, display);
	
	//  ??? set numlockmask 
	XModifierKeymap *modmap = XGetModifierMapping(display);
	for (int k = 0; k < 8; k++) {
		for (int j = 0; j < modmap->max_keypermod; j++) {
			if (modmap->modifiermap[modmap->max_keypermod*k + j] == XKeysymToKeycode(display, XK_Num_Lock)) {
				numlockmask = (1 << k);
			}
		}
	}
	
	XFreeModifiermap(modmap);
	
	unknowwm_client_event     = XInternAtom(display, "UNKNOWWM_CLIENT_EVENT",    false);

	wmatoms[WM_PROTOCOLS]     = XInternAtom(display, "WM_PROTOCOLS",             false);
	wmatoms[WM_DELETE_WINDOW] = XInternAtom(display, "WM_DELETE_WINDOW",         false);

	netatoms[NET_SUPPORTED]   = XInternAtom(display, "_NET_SUPPORTED",           false);
	netatoms[NET_WM_STATE]    = XInternAtom(display, "_NET_WM_STATE",            false);
	netatoms[NET_ACTIVE]      = XInternAtom(display, "_NET_ACTIVE_WINDOW",       false);
	netatoms[NET_FULLSCREEN]  = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", false);

	XChangeProperty(display, rootWin, netatoms[NET_SUPPORTED], XA_ATOM, 32,
		            PropModeReplace, (unsigned char *)netatoms, NET_COUNT);


	XSetErrorHandler(WMDetected);
	XSelectInput(display, rootWin, rootMask());
	XSync(display, False);
	XSetErrorHandler(XError);
	XSync(display, False);

	grabKeys();
	if (this->config->DEFAULT_DESKTOP >= 0 && this->config->DEFAULT_DESKTOP < this->config->DESKTOPS) {
		Argument arg;
		arg.i = this->config->DEFAULT_DESKTOP;
		changeDesktop(&arg);
	}
	if (this->config->DEFAULT_MONITOR >= 0 && this->config->DEFAULT_MONITOR < monitorCount) {
		Argument arg;
		arg.i = this->config->DEFAULT_MONITOR;
		changeMonitor(&arg);
	}
}

template <std::size_t N>
int execvp(const char* file, const char* const (&argv)[N]) {
	assert((N > 0) && (argv[N - 1] == nullptr));
	return execvp(file, const_cast<char* const*>(argv));
}

void WindowManager::runCmd(const Argument *arg) {
	if (fork()) {
		return;
	}
	if (display) {
		close(ConnectionNumber(display));
	}
	setsid();
	auto sz = arg->com.size()+1;
	char * a[sz];
	for(unsigned int i = 0; i < sz-1; i++) {
		//LINFO << arg->com[i] << " RUNCMD\n"; 
		a[i] = (char*)(arg->com[i].c_str());
	}
	a[sz-1] = nullptr;
	execvp((char*)(arg->com[0].c_str()), a);
}

void WindowManager::swapMaster(const Argument *arg) {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId].get();
	if (!d->getCur() || d->clients.size() <= 1) {
		return;
	}
	auto _id1 = d->getCur()->id;
	auto _id2 = 0;
	
	d->clients[_id1]->id = _id2;
	d->clients[_id2]->id = _id1;

	auto elem = std::move(d->clients[_id1]);
	d->clients[_id1] = std::move(d->clients[_id2]);
	d->clients[_id2] = std::move(elem);

	d->prevClientId = d->clients.size()-1;
	d->curClientId = 0;
	focus(d->getHead(), d, monitors[monitorId].get());
	
	if (!d->getCur()->isFloat && !d->getCur()->isTrans) {
		tile(d, monitors[monitorId].get());
	}
}

void WindowManager::switchMode(const Argument* Argument) {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId].get();
	if (d->mode != Argument->i) {
		d->mode = Argument->i;
	}
	for (auto i = 0; i < d->clients.size(); i++) {
		d->clients[i]->isHide = false;
	}
	if (d->mode != FLOAT_MODE) {
		for (auto i = 0; i < d->clients.size(); i++) {
			d->clients[i]->isFloat = d->clients[i]->isFull = false;
		}
	}
	if (d->clients.size() > 0) {
		tile(d, monitors[monitorId].get());
		focus(d->getCur() != nullptr ? d->getCur() : d->getHead(), d, monitors[monitorId].get());
	}
	desktopInfo();
}

void WindowManager::togglePanel(const Argument *arg) {
	Monitor *m = monitors[monitorId].get();
	m->desktops[m->desktopCurId]->isBar = !m->desktops[m->desktopCurId]->isBar;
	tile(m->desktops[m->desktopCurId].get(), m);
}

void WindowManager::unmapNotify(XEvent *e) {
	Monitor *m = nullptr; 
	Desktop *d = nullptr; 
	Client *c = nullptr;
	if (winToClient(e->xunmap.window, &c, &d, &m)) {
		if (c->isUnmap) {
			return;
		}
		if (config->SHOW_DECORATE && c->isDecorated) {
			c->decorationsDestroy(display);
		}
		removeClient(c, d, m);
	}
}

bool WindowManager::winToClient(Window w, Client **c, Desktop **d, Monitor **m, bool isTitle) {
	for (int cm = 0; cm < monitorCount; cm++) {
		*m = monitors[cm].get();
		for (int cd = 0; cd < config->DESKTOPS; cd++) {
			*d = (*m)->desktops[cd].get();
			for (int i = 0; i < (*d)->clients.size(); i++) {
				*c = (*d)->clients[i].get();
				if (!isTitle && *c && (*c)->win == w) {
					return (*c != nullptr);
				} else if (isTitle && *c && (*c)->decorate == w) {
					return (*c != nullptr);
				}
			}
		}
	}
	return false;
}

WindowManager::~WindowManager() {
	XCloseDisplay(display);
}

void WindowManager::prevWin(const Argument *arg) {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId].get();
	if (d->getCur() && d->clients.size() > 1) {
		auto c = (d->getCur()->id - 1 >= 0) ? d->clients[d->getCur()->id - 1].get() : 
			d->clients[d->clients.size()-1].get();
		if (c->isHide) {
			hideClient(c, monitors[monitorId].get());
		}
		focus(c, d, monitors[monitorId].get());
	}
}

void WindowManager::quit(const Argument *arg) {
	retval = arg->i;
	isRunning = false;
}

void WindowManager::removeClient(Client *c, Desktop *d, Monitor *m) {
	if (!c || c->id >= d->clients.size()) {
		return;
	}

	bool isSame = d->getCur() == c;

	d->removeCurClient(c);
	if (isSame || (d->getHead() && d->clients.size() > 1)) {
		focus(d->getHead(), d, m);
	}
	tile(d, m);
	desktopInfo();
}

void WindowManager::resizeMaster(const Argument *Argument) {
	Monitor *m = monitors[monitorId].get();
	Desktop *d = m->desktops[m->desktopCurId].get();
	int msz = (d->mode == H_STACK_UP || d->mode == H_STACK_DOWN ? m->h : m->w) * config->MASTER_SIZE + (d->masterSize += Argument->i);
	if (msz >= config->MINWSZ && (d->mode == H_STACK_UP || d->mode == H_STACK_DOWN ? m->h : m->w) - msz >= config->MINWSZ) {
		tile(d, m);
	} else {
		d->masterSize -= Argument->i;
	}
}

void WindowManager::resizeStack(const Argument *Argument) {
	monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId]->firstStackSize += Argument->i;
	tile(monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId].get(), monitors[monitorId].get());
}

void WindowManager::nextDesktop(const Argument *arg) {
	Argument _arg;
	_arg.i = (config->DESKTOPS + monitors[monitorId]->desktopCurId + arg->i) % config->DESKTOPS;
	changeDesktop(&_arg);
}

void WindowManager::nextFilledDesktop(const Argument *arg) {
	Monitor *m = monitors[monitorId].get();
	int n = arg->i;
	while (n < config->DESKTOPS && !m->desktops[(config->DESKTOPS + m->desktopCurId + n) % config->DESKTOPS]->getHead()) {
		(n += arg->i);
	}
	Argument _arg;
	_arg.i = (config->DESKTOPS + m->desktopCurId + n) % config->DESKTOPS;
	changeDesktop(&_arg);
}


void WindowManager::run() {
	XEvent ev;
	while(isRunning && !XNextEvent(display, &ev)) {
		if (events[ev.type]) {
			(*this.*events[ev.type])(&ev);
		}
	}
}

int WindowManager::WMDetected(Display* display, XErrorEvent* e) {
	LERR << "Another WM already running\n";
	exit(-1);
	return 0;
}

int WindowManager::XError(Display* display, XErrorEvent* e) { 
	if ((e->error_code == BadAccess &&
		(e->request_code == X_GrabKey || e->request_code == X_GrabButton)) ||
		(e->error_code  == BadMatch &&
		(e->request_code == X_SetInputFocus || e->request_code == X_ConfigureWindow)) ||
		(e->error_code  == BadDrawable &&
		(e->request_code == X_PolyFillRectangle || e->request_code == X_CopyArea ||
		e->request_code == X_PolySegment || e->request_code == X_PolyText8))
		|| e->error_code   == BadWindow) {
		
		return 0;
	}
}

void WindowManager::reloadConfig(const Argument *arg) {
	config->initDefault();
	if (arg->com[0] != "") {
		config->parse(arg->com[0]);
	}
	Argument dummyArg(0);
	restartMonitors(&dummyArg);
}

void WindowManager::hideClient(Client *c, Monitor *m) {
	if (c == nullptr) {
		return;
	}
	auto win = config->SHOW_DECORATE && c->isDecorated ? (c)->decorate : (c)->win;
	
	XWindowAttributes wa;
	if (XGetWindowAttributes(display, win, &wa)) {
		
		if (c->isHide) {
			XMoveResizeWindow(display, win, c->hideX, c->hideY, wa.width, wa.height);
			if (c->isDecorated) {
				c->moveResizeLocal(c->hideX, c->hideY, wa.width, wa.height, *config, display);
			}
		} else {
			c->hideX = wa.x;
			c->hideY = wa.y;
			XMoveResizeWindow(display, win, - 2 * m->w, 0, wa.width, wa.height);
			if (c->isDecorated) {
				c->moveResizeLocal(- 2 * m->w, 0, wa.width, wa.height, *config, display);
			}
		}
		c->isHide = !c->isHide;
		
	}
}

void WindowManager::hideCurClient(const Argument *arg) {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId].get();
	Client *c = d->getCur();
	if (c == nullptr) {
		return;
	}
	hideClient(c, monitors[monitorId].get());
}

void WindowManager::hideAllClientOnDescktop(const Argument *arg) {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId].get();
	bool hide = true;
	for (auto& _c : d->clients) {
		if (!_c->isHide) {
			hide = false;
			break;
		}
	}
	if (hide) {
		d->isHide = false;
		for (auto& _c : d->clients) {
			if (_c->isHide) {
				hideClient(_c.get(), monitors[monitorId].get());
			}
		}
	} else {
		d->isHide = true;
		for (auto& _c : d->clients) {
			if (!_c->isHide) {
				hideClient(_c.get(), monitors[monitorId].get());
			}
		}
	}
}

void WindowManager::changeDecorateBorder(const Argument *arg) {
	config->DECORATE_BORDER_WIDTH += arg->i;
	config->DECORATE_BORDER_WIDTH  = config->DECORATE_BORDER_WIDTH < 0 ? 0 : config->DECORATE_BORDER_WIDTH;
	config->DECORATE_BORDER_WIDTH  = config->DECORATE_BORDER_WIDTH > 100 ? 1000 : config->DECORATE_BORDER_WIDTH;
	Monitor *m = monitors[monitorId].get();
	Desktop *d = m->desktops[m->desktopCurId].get();
	Client  *c = d->getCur();
	tile(d, m);
	focus(c, d, m);
}

void WindowManager::changeBorder(const Argument *arg) {
	config->BORDER_WIDTH += arg->i;
	config->BORDER_WIDTH  = config->BORDER_WIDTH < 0 ? 0 : config->BORDER_WIDTH;
	config->BORDER_WIDTH  = config->BORDER_WIDTH > 100 ? 1000 : config->BORDER_WIDTH;
	Monitor *m = monitors[monitorId].get();
	Desktop *d = m->desktops[m->desktopCurId].get();
	Client  *c = d->getCur();
	tile(d, m);
	focus(c, d, m);
}

void WindowManager::changeGap(const Argument *arg) {
	config->USELESSGAP += arg->i;
	config->USELESSGAP  = config->USELESSGAP < 0 ? 0 : config->USELESSGAP;
	config->USELESSGAP  = config->USELESSGAP > 100 ? 1000 : config->USELESSGAP;
	Monitor *m = monitors[monitorId].get();
	Desktop *d = m->desktops[m->desktopCurId].get();
	Client  *c = d->getCur();
	tile(d, m);
	focus(c, d, m);
}

void WindowManager::addMaster(const Argument *arg) {
	Monitor *m =  monitors[monitorId].get();
	Desktop *d = m->desktops[monitors[monitorId]->desktopCurId].get();
	d->nm += arg->i;
	d->nm = d->nm <= 0 ? config->NMASTER : d->nm;
	tile(d, m);
}
