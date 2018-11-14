/*
UNKNOWN WM
*/

#include "wm.h"
#include <unistd.h>
#include "logger.h"
#include <algorithm>

std::unique_ptr<WindowManager> WindowManager::Create() {
	Display* display = XOpenDisplay(nullptr);
	if (display == nullptr) {
		Logger::Err("Failed to open display");
		return nullptr;
	}
	return std::unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display* display): display(display) {}

Client* WindowManager::AddWindow(Window w, Desktop *d) {
	/*std::string title = "";
	XTextProperty name;
	XGetTextProperty(display, w, &name, XA_WM_NAME);
	if (name.nitems && name.encoding == XA_STRING) {
		title = (char*)name.value;
	}
	if (name.value) {
		XFree(name.value);
	}
	
	auto it = std::find(config->ignoreApps.begin(), config->ignoreApps.end(), title);
	bool isIgnoreWin = false;
	if (it != config->ignoreApps.end()) {
		isIgnoreWin = true;
	}
*/
	Client *c = new Client;
	//if (isIgnoreWin) c->isIgnore = true;
	//c->title = title;
	if (d->clients.size() == 0) {
		c->id = 0;
		d->clients.push_back(c);
		d->curClientId = 0;
		d->prevClientId = 0;
	} else if (!config->ATTACH_ASIDE) {
		d->clients.insert(d->clients.begin(), c);
		c->id = 0;
		d->curClientId = 0;
		for (auto i = 0; i < d->clients.size(); i++) {
			d->clients[i]->id = i;
		}
	} else {
		c->id = d->clients.size();
		d->clients.push_back(c);
		d->curClientId = c->id;
	}
	c->win = w;
	if (config->SHOW_DECORATE) {
		c->decorate = XCreateSimpleWindow(display, rootWin, 0, 0, 100, 100, config->BORDER_WIDTH,
			                              winFocus, decorateWinFocus);
		c->isDecorated = true;
		XWindowAttributes wa;
		if (XGetWindowAttributes(display, c->win, &wa)) {
			XMoveResizeWindow(display, c->decorate, wa.x, wa.y, wa.width, wa.height);
			MoveResizeLocal(c->win, wa.x, wa.y, wa.width, wa.height);
		}
	}
	XSelectInput(display, c->win, 
		         PropertyChangeMask | FocusChangeMask | (config->FOLLOW_MOUSE?EnterWindowMask:0));
	return c;
}

void WindowManager::ChangeLayout(const Argument *arg) {
	Monitor *m =  monitors[monitorId];
	Desktop *d = m->desktops[monitors[monitorId]->desktopCurId];
	auto ls = config->layouts;
	if (config->desktopLayouts.find(monitors[monitorId]->desktopCurId) != config->desktopLayouts.end()) {
		ls = config->desktopLayouts[monitors[monitorId]->desktopCurId];
	}
	d->layoutId = ((d->layoutId + arg->i) < 0 ? ls.size()-1 : (d->layoutId + arg->i)) % ls.size();
	Argument _arg(ls[d->layoutId]);
	SwitchMode(&_arg);
}

void WindowManager::buttonPress(XEvent *e) {
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;

	Bool w = WinToClient(e->xbutton.window, &c, &d, &m);
	int cm = 0; 
	while(m != monitors[cm] && cm < monitorCount) {
		cm++;
	}
	if (w && config->CLICK_TO_FOCUS && e->xbutton.button == config->FOCUS_BUTTON && 
		(c != d->GetCur()|| cm != monitorId)) {
		if (cm != monitorId) {
			Argument _arg;
			_arg.i = cm;
			ChangeMonitor(&_arg);
		}
		Focus(c, d, m);
	}
	for (unsigned int i = 0; i < config->buttons.size(); i++) {
		if (CleanMask(config->buttons[i].mask) == CleanMask(e->xbutton.state) &&
			config->buttons[i].func != "" && config->buttons[i].button == e->xbutton.button) {
			if (w && cm != monitorId) {
				Argument _arg;
				_arg.i = cm;
				ChangeMonitor(&_arg);
			}
			if (w && c != d->GetCur()) {
				Focus(c, d, m);
			}
			RunFunc("button", config->buttons[i].func, &(config->buttons[i].arg));
		}
	}
}

void WindowManager::ChangeDesktop(const Argument *arg) {
	Monitor *m = monitors[monitorId];
	if (arg->i == m->desktopCurId || arg->i < 0 || arg->i >= config->DESKTOPS) {
		return;
	}
	Desktop *d = m->desktops[(m->desktopPrevId = m->desktopCurId)];
	Desktop *n = m->desktops[(m->desktopCurId = arg->i)];
	
	auto _d = n->GetCur();
	if (_d != nullptr){
		if (config->SHOW_DECORATE && _d->isDecorated) {
			XMapWindow(display, _d->decorate);
		}
		XMapWindow(display, _d->win);
	}
	for (auto c = n->clients.begin(); c != n->clients.end(); c++) {
		(*c)->isUnmap = false;
		if (config->SHOW_DECORATE && (*c)->isDecorated) {
			XMapWindow(display, (*c)->decorate);
		}
		XMapWindow(display, (*c)->win);
	}
	XSetWindowAttributes attr1;
	attr1.do_not_propagate_mask = SubstructureNotifyMask;
	XChangeWindowAttributes(display, rootWin, CWEventMask, &attr1);
	for (auto c = d->clients.begin(); c != d->clients.end(); c++) {
		(*c)->isUnmap = true;
		if (*c != (d->GetCur())) {
			if (config->SHOW_DECORATE && (*c)->isDecorated) {
				XUnmapWindow(display, (*c)->decorate);
			}
			XUnmapWindow(display, (*c)->win);
		}
	}
	if (d->GetCur() != nullptr) {
		if (config->SHOW_DECORATE && d->GetCur()->isDecorated) {
			XUnmapWindow(display, d->GetCur()->decorate);
		}
		XUnmapWindow(display, d->GetCur()->win);
	}
	XSetWindowAttributes attr2;
	attr2.event_mask = RootMask();
	XChangeWindowAttributes(display, rootWin, CWEventMask, &attr2);
	
	if (n->GetCur() != nullptr) { 
		Tile(n, m);
		Focus(n->GetCur(), n, m);
	}
	DesktopInfo();
}

void WindowManager::ChangeMonitor(const Argument *arg) {
	if (arg->i == monitorId || arg->i < 0 || arg->i >= monitorCount) {
		return;
	}
	Monitor *m = monitors[monitorId];
	monitorId = arg->i;
	Monitor *n = monitors[monitorId];
	Client *mClient = m->desktops[m->desktopCurId]->GetCur();
	Client *nClient = n->desktops[n->desktopCurId]->GetCur();
	Focus(mClient, m->desktops[m->desktopCurId], m);
	Focus(nClient, n->desktops[n->desktopCurId], n);
	DesktopInfo();
}

void WindowManager::Focus(Client *c, Desktop *d, Monitor *m) {
	if (!d->GetHead() || !c) {
		XDeleteProperty(display, rootWin, netatoms[NET_ACTIVE]);
		d->curClientId = -1;
		d->clients.clear();
		return;
	} else if (d->GetCur() != c) {
		d->prevClientId = d->GetCur() ? d->GetCur()->id : -1; 
		d->curClientId = c->id;
	}
	int n = 0, fl = 0, ft = 0;
	for (auto _c = d->clients.begin(); _c != d->clients.end(); _c++, n++) {
		if (IsFloatOrFullscreen(*_c)) {
			fl++;
			if (!(*_c)->isFull) {
				ft++;
			}
		}
	}
	
	int scale = config->SHOW_DECORATE ? 2 : 1;
	Window w[n*scale];
	auto _id = (d->GetCur()->isFloat || d->GetCur()->isTrans) ? 0 : ft;
	w[_id*scale] = d->GetCur()->win;
	if (config->SHOW_DECORATE) {
		w[_id*scale+1] = d->GetCur()->decorate;
	}
	fl += !IsFloatOrFullscreen(d->GetPrev()) ? 1 : 0;
	
	for (auto _c = d->clients.begin(); _c != d->clients.end(); _c++) {
		auto win = config->SHOW_DECORATE && (*_c)->isDecorated ? (*_c)->decorate : (*_c)->win;
		if (config->SHOW_DECORATE) {
			//XClearWindow(display, win);
			XSetWindowBackground(display, win, 
				                 ((*_c) != d->GetCur()) ? 
				                 decorateWinUnfocus : 
				                 (m == monitors[monitorId]) ? 
				                 decorateWinFocus : 
				                 decorateWinInfocus);
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
				XftDrawString8 (draw, &msgcolor, msgfont, config->TITLE_DX, config->TITLE_DY, 
					(XftChar8*)(*_c)->title.c_str(), 
					(*_c)->title.size());
			}
			if (config->TITLE_POSITION == TITLE_DOWN) {
				XWindowAttributes wa;
				if (XGetWindowAttributes(display, win, &wa)) {
					XftDrawString8 (draw, &msgcolor, msgfont, 
						config->TITLE_DX, 
						wa.height-config->TITLE_DY, 
						(XftChar8*)(*_c)->title.c_str(), 
						(*_c)->title.size());
				}
				
			}
			if (config->TITLE_POSITION == TITLE_LEFT) {
				XWindowAttributes wa;
				if (XGetWindowAttributes(display, win, &wa)) {
					for (auto i = 0; i < (*_c)->title.size(); ++i) {
						std::string s(1, (*_c)->title[i]);
						XftDrawString8 (draw, &msgcolor, msgfont, 
							config->TITLE_DX, 
							config->TITLE_DY + i*msgfont->height, 
							(XftChar8*)s.c_str(), 1);
					}
				}
				
			}
			if (config->TITLE_POSITION == TITLE_RIGHT) {
				XWindowAttributes wa;
				if (XGetWindowAttributes(display, win, &wa)) {
					for (auto i = 0; i < (*_c)->title.size(); ++i) {
						std::string s(1, (*_c)->title[i]);
						XftDrawString8 (draw, &msgcolor, msgfont, 
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
		XSetWindowBorder(display, win, ((*_c) != d->GetCur()) ? winUnfocus : (m == monitors[monitorId]) ? winFocus : winInfocus);

		XSetWindowBorderWidth(display, win, (*_c)->isFull || (!IsFloatOrFullscreen(*_c) &&
			(d->mode == MONOCLE || !d->clients.size() > 1)) ? 0 : config->BORDER_WIDTH);

		

		if (*_c != d->GetCur()) {
			auto id = ((*_c)->isFull ? --fl:IsFloatOrFullscreen(*_c) ? --ft:--n) * scale;
			w[id] = (*_c)->win;
			if (config->SHOW_DECORATE) {
				w[id+1] = (*_c)->decorate;
			}
		}
		if (config->CLICK_TO_FOCUS || (*_c) == d->GetCur()) {
			GrabButtons(*_c);
		}
	}
	XRestackWindows(display, w, Length(w));

	XSetInputFocus(display, d->GetCur()->win, RevertToPointerRoot, CurrentTime);
	XChangeProperty(display, rootWin, netatoms[NET_ACTIVE], XA_WINDOW, 32,
		            PropModeReplace, (unsigned char *)&d->GetCur()->win, 1);
	XSync(display, false);
}

void WindowManager::Cleanup(void) {
	Window rootReturn, parentReturn, *children;
	unsigned int childrenCount;

	XUngrabKey(display, AnyKey, AnyModifier, rootWin);
	XQueryTree(display, rootWin, &rootReturn, &parentReturn, &children, &childrenCount);
	for (unsigned int i = 0; i < childrenCount; i++) {
		DeleteWindow(children[i]);
	}
	if (children) {
		XFree(children);
	}
	XSync(display, false);

	for (auto it = monitors.begin() ; it != monitors.end(); ++it) {
		delete (*it);
	}
	monitors.clear();
}

void WindowManager::ClientToDesktop(const Argument *arg) {
	Monitor *m = monitors[monitorId];
	Desktop *d = m->desktops[m->desktopCurId];
	Desktop *n = nullptr;
	if (arg->i == m->desktopCurId || arg->i < 0 || arg->i >= config->DESKTOPS || !d->GetCur()) {
		return;
	}
	Client *c = d->GetCur();
	c->isUnmap = true;
	n = m->desktops[arg->i];
	d->clients.erase(d->clients.begin()+c->id);
	int i = 0;
	for (auto _c = d->clients.begin(); _c != d->clients.end(); _c++) {
		(*_c)->id = i;
		i++;
	}
	
	XSetWindowAttributes _attr1;
	_attr1.do_not_propagate_mask = SubstructureNotifyMask;
	XChangeWindowAttributes(display, rootWin, CWEventMask, &_attr1);
	if (XUnmapWindow(display, c->win)) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			XUnmapWindow(display, c->decorate);
		}
		Focus(d->GetHead(), d, m);
	}
	XSetWindowAttributes _attr2;
	_attr2.event_mask = RootMask();
	XChangeWindowAttributes(display, rootWin, CWEventMask, &_attr2);
	if (!(c->isFloat || c->isTrans) || (d->GetHead() && d->clients.size() > 1)) {
		Tile(d, m);
	}
	c->id = n->clients.size();
	n->clients.push_back(c);
	Focus(c, n, m);
	if (config->FOLLOW_WINDOW) {
		ChangeDesktop(arg);
	} else {
		DesktopInfo();
	}
}

void WindowManager::ClientToMonitor(const Argument *arg) {
	Monitor *cm = monitors[monitorId];
	Monitor *nm = nullptr;
	Desktop *cd = cm->desktops[cm->desktopCurId];
	Desktop *nd = nullptr;
	if (arg->i == monitorId || arg->i < 0 || arg->i >= monitorCount || !cd->GetCur()) {
		return;
	}
	nd = monitors[arg->i]->desktops[(nm = monitors[arg->i])->desktopCurId];
	Client *c = cd->GetCur();
	cd->clients.erase(cd->clients.begin()+cd->GetCur()->id);
	int i = 0;
	for (auto _c = cd->clients.begin(); _c != cd->clients.end(); _c++) {
		(*_c)->id = i;
		i++;
	}

	Focus(cd->GetHead(), cd, cm);
	if (!(c->isFloat || c->isTrans) || (cd->GetHead() && cd->clients.size() > 1)) {
		Tile(cd, cm);
	}
	if (IsFloatOrFullscreen(c)) {
		c->isFloat = c->isFull = false;
	}
	nd->clients.push_back(c);
	Focus(c, nd, nm);
	Tile(nd, nm);
	if (config->FOLLOW_MONITOR) {
		ChangeMonitor(arg);
	} else {
		DesktopInfo();
	}
}

void WindowManager::clientMessage(XEvent *e) {
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;
	if (!WinToClient(e->xclient.window, &c, &d, &m)) {
		return;
	}
	if (e->xclient.message_type        == netatoms[NET_WM_STATE] && (
		(unsigned)e->xclient.data.l[1] == netatoms[NET_FULLSCREEN] ||
		(unsigned)e->xclient.data.l[2] == netatoms[NET_FULLSCREEN])) {
		FullscreenMode(c, d, m, (e->xclient.data.l[0] == 1 || (e->xclient.data.l[0] == 2 && !c->isFull)));
		if (!(c->isFloat || c->isTrans) || !d->GetHead()->next) {
			Tile(d, m);
		}
	} else if (e->xclient.message_type == netatoms[NET_ACTIVE]) {
		Focus(c, d, m);
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
	if (WinToClient(ev->window, &c, &d, &m)) {
		Tile(d, m);
	}
}

void WindowManager::DeleteWindow(Window w) {
	XEvent ev = { .type = ClientMessage };
	ev.xclient.window = w;
	ev.xclient.format = 32;
	ev.xclient.message_type = wmatoms[WM_PROTOCOLS];
	ev.xclient.data.l[0]    = wmatoms[WM_DELETE_WINDOW];
	ev.xclient.data.l[1]    = CurrentTime;
	XSendEvent(display, w, false, NoEventMask, &ev);
}

void WindowManager::DesktopInfo() {
	Monitor *m = nullptr;
	Client *c = nullptr;
	bool urgent = false;

	std::string info = "{\n";

	for (int cm = 0; cm < monitorCount; cm++) {
		info += "\"m" + std::to_string(cm) + "\" : {";
		for (int cd = 0; cd < config->DESKTOPS; cd++) {
			m = monitors[cm];
			info += "\"d" + std::to_string(cd) + "\" : {";
			auto cc = 0;
			for (auto _c = m->desktops[cd]->clients.begin(); _c != m->desktops[cd]->clients.end(); _c++) {
				info += "\"" + (*_c)->title + "\" : ";
				if (m->desktops[cd]->curClientId == (*_c)->id) {
					info += "true";
				} else {
					info += "false";
				}
				if (cc != m->desktops[cd]->clients.size()) {
					info += ",\n";
				}
				cc++;
			}
			info += "}\n";
			if (cd != config->DESKTOPS-1) {
				info += ",\n";
			}
		}
		info += "}\n";
		if (cm != monitorCount-1) {
			info += ",\n";
		}
	}
	info += "}\n";
	Logger::Log(info);
}

void WindowManager::destroyNotify(XEvent *e) {
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;
	if (WinToClient(e->xdestroywindow.window, &c, &d, &m)) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			DecorationsDestroy(c);
		}
		RemoveClient(c, d, m);
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
		!WinToClient(e->xcrossing.window, &c, &d, &m) || 
		e->xcrossing.window == d->GetCur()->win) {
		return;
	}
	if (m != monitors[monitorId]) {
		for (int cm = 0; cm < monitorCount; cm++) {
			if (m == monitors[cm]) {
				Argument _arg;
				_arg.i = cm;
				ChangeMonitor(&_arg);
			}
		}
	}
	if ((p = d->GetPrev())) {
		XSetWindowAttributes attr;
		attr.do_not_propagate_mask = EnterWindowMask;
		XChangeWindowAttributes(display, p->win, CWEventMask, &attr);
	}
	Focus(c, d, m);
	if (p) {
		XSetWindowAttributes attr;
		attr.event_mask = EnterWindowMask;
		XChangeWindowAttributes(display, p->win, CWEventMask, &attr);
	}
}

void WindowManager::focusIn(XEvent *e) {
	Monitor *m = monitors[monitorId];
	Desktop *d = m->desktops[m->desktopCurId];
	if (d->GetCur() && d->GetCur()->win != e->xfocus.window) {
		Focus(d->GetCur(), d, m);
	}
}


void WindowManager::FocusUrGent() {
	Monitor *m = monitors[monitorId];
	Client *c = nullptr;
	int d = -1;
	auto _c = m->desktops[m->desktopCurId]->clients.begin();
	for (_c = m->desktops[m->desktopCurId]->clients.begin(); 
		_c != m->desktops[m->desktopCurId]->clients.end() && !(*_c)->isUrgn; _c++);
	c = *_c;
	while (!c && d < config->DESKTOPS-1) {
		for (auto _c = m->desktops[++d]->clients.begin(); 
			_c != m->desktops[m->desktopCurId]->clients.end() && !(*_c)->isUrgn; c++);
		c = *_c;
	}
	if (c) {
		if (d != -1) {
			Argument _arg;
			_arg.i = d;
			ChangeDesktop(&_arg);
			Focus(c, m->desktops[m->desktopCurId], m); 
		}
	}
}

unsigned long WindowManager::GetColor(std::string _color, const int screen) {
	const char* color = _color.c_str();
	XColor c;
	Colormap map = DefaultColormap(display, screen);
	if (!XAllocNamedColor(display, map, color, &c, &c)) {
		Logger::Err("Cannot allocate color\n");
		exit(-1);
	}
	return c.pixel;
}


void WindowManager::GrabButtons(Client *c) {
	Monitor *cm = monitors[monitorId];
	unsigned int b, m, modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };

	for (m = 0; config->CLICK_TO_FOCUS && m < Length(modifiers); m++) {
		if (c != cm->desktops[cm->desktopCurId]->GetCur()) {
			XGrabButton(display, config->FOCUS_BUTTON, modifiers[m],
				c->win, false, ButtonMask(), GrabModeAsync, GrabModeAsync, None, None);
		} else {
			XUngrabButton(display, config->FOCUS_BUTTON, modifiers[m], c->win);
		}
	}
	for (b = 0, m = 0; b < config->buttons.size(); b++, m = 0) {
		while (m < Length(modifiers)) {
			XGrabButton(display, config->buttons[b].button, 
				        config->buttons[b].mask|modifiers[m++], c->win,
				        false, ButtonMask(), GrabModeAsync, GrabModeAsync, None, None);
		}
	}
}


void WindowManager::GrabKeys() {
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
			CleanMask(config->keys[i].mod) == CleanMask(e->xkey.state)) {
			if (config->keys[i].func != "") {
				RunFunc("keys", config->keys[i].func, &config->keys[i].arg);
				//config->keys[i].func(&config->keys[i].arg);
			}
		}
	}
}

void WindowManager::RunFunc(std::string type, std::string funcStr, const Argument *arg) {
	if (type == "keys") {
		if (funcStr == "TogglePanel") {
			TogglePanel();
		}
		
		if (funcStr == "SwapMaster") {
			SwapMaster();
		}

		if (funcStr == "Quit") {
			Quit(arg);
		}
		if (funcStr == "RunCmd") {
			RunCmd(arg);
		}
		if (funcStr == "KillClient") {
			KillClient();
		}
		if (funcStr == "NextWin") {
			NextWin();
		}
		if (funcStr == "PrevWin") {
			PrevWin();
		}
		if (funcStr == "MoveResize") {
			MoveResize(arg);
		}
		if (funcStr == "SwitchMode") {
			SwitchMode(arg);
		}
		if (funcStr == "ResizeMaster") {
			ResizeMaster(arg);
		}
		if (funcStr == "ResizeStack") {
			ResizeStack(arg);
		}
		if (funcStr == "MoveDown") {
			MoveDown();
		}
		if (funcStr == "MoveUp") {
			MoveUp();
		}
		if (funcStr == "NextDesktop") {
			NextDesktop(arg);
		}
		if (funcStr == "NextFilledDesktop") {
			NextFilledDesktop(arg);
		}
		if (funcStr == "PrevDesktop") {
			PrevDesktop();
		}
		if (funcStr == "ClientToDesktop") {
			ClientToDesktop(arg);
		}

		if (funcStr == "ToggleFloatClient") {
			ToggleFloatClient();
		}
		if (funcStr == "ToggleFullscreenClient") {
			ToggleFullscreenClient();
		}

		if (funcStr == "ChangeDecorateBorder") {
			ChangeDecorateBorder(arg);
		}
		if (funcStr == "ChangeBorder") {
			ChangeBorder(arg);
		}
		if (funcStr == "ChangeGap") {
			ChangeGap(arg);
		}
		if (funcStr == "AddMaster") {
			AddMaster(arg);
		}
		if (funcStr == "HideCurClient") {
			HideCurClient();
		}
		if (funcStr == "HideAllClientOnDescktop") {
			HideAllClientOnDescktop();
		}
		if (funcStr == "ChangeDesktop") {
			ChangeDesktop(arg);
		}
		if (funcStr == "ChangeLayout") {
			ChangeLayout(arg);
		}
	}
	if (type =="button") {
		if (funcStr == "MouseMotion") {
			MouseMotion(arg);
		}
	}
}

void WindowManager::KillClient(void) {
	Monitor *m = monitors[monitorId];
	Desktop *d = m->desktops[m->desktopCurId];
	if (!d->GetCur()) {
		return;
	}
	Atom *prot = nullptr;
	int n = -1;
	if (XGetWMProtocols(display, d->GetCur()->win, &prot, &n)) {
		while(--n >= 0 && prot[n] != wmatoms[WM_DELETE_WINDOW]);
	}
	if (config->SHOW_DECORATE) {
		DecorationsDestroy(d->GetCur());
	}
	if (n < 0) { 
		XKillClient(display, d->GetCur()->win);
		RemoveClient(d->GetCur(), d, m); 
	} else {
		DeleteWindow(d->GetCur()->win);
	}
	
	if (prot) {
		XFree(prot);
	}
}

void WindowManager::GridMode(int x, int y, int w, int h, Desktop *d) {
	int n = 0;
	int cols = 0;
	int cn = 0;
	int rn = 0;
	int i = -1;
	
	for (auto c = d->clients.begin(); c != d->clients.end(); c++) {
		if (!IsFloatOrFullscreen(*c)) {
			++n;
		}
	}

	for (cols = 0; cols <= n/2; cols++) {
		if (cols*cols >= n) {
			break;
		}
	}
	if (n == 0) {
		return;
	} else if (n == 5) {
		cols = 2;
	}

	int rows = n/cols;
	int ch = h - config->BORDER_WIDTH - config->USELESSGAP;
	int cw = (w - config->BORDER_WIDTH - config->USELESSGAP)/(cols ? cols:1);
	for (auto c = d->clients.begin(); c != d->clients.end(); c++) {
		if (IsFloatOrFullscreen(*c)) {
			continue; 
		} else {
			++i;
		}
		if (i/rows + 1 > cols - n % cols) {
			rows = n/cols + 1;
		}
		auto win = config->SHOW_DECORATE ? (*c)->decorate : (*c)->win;
		if (!(*c)->isHide){
		XMoveResizeWindow(display, win, x + cn*cw + config->USELESSGAP, 
			y + rn*ch/rows + config->USELESSGAP, 
			cw - 2*config->BORDER_WIDTH-config->USELESSGAP, 
			ch/rows - 2*config->BORDER_WIDTH-config->USELESSGAP);
		
		if (config->SHOW_DECORATE && (*c)->isDecorated) {
			MoveResizeLocal((*c)->win, x + cn*cw + config->USELESSGAP, 
				y + rn*ch/rows + config->USELESSGAP, 
				cw - 2*config->BORDER_WIDTH-config->USELESSGAP, 
				ch/rows - 2*config->BORDER_WIDTH-config->USELESSGAP);
		}
		}
		if (++rn >= rows) {
			rn = 0;
			cn++;
		}
	}
}

void WindowManager::MoveResizeLocal(Window win , int x, int y, int w, int h) {
	int titleup = (config->SHOW_TITLE && config->TITLE_POSITION == TITLE_UP) ? config->TITLE_HEIGHT:0;
	int titledown = (config->SHOW_TITLE && config->TITLE_POSITION == TITLE_DOWN) ? config->TITLE_HEIGHT:0;
	int titleleft = (config->SHOW_TITLE && config->TITLE_POSITION == TITLE_LEFT) ? config->TITLE_HEIGHT:0;
	int titleright = (config->SHOW_TITLE && config->TITLE_POSITION == TITLE_RIGHT) ? config->TITLE_HEIGHT:0;
	int titleh = config->SHOW_TITLE ? config->TITLE_HEIGHT:0;
	XMoveResizeWindow(display, win, 
		x + config->BORDER_WIDTH + config->DECORATE_BORDER_WIDTH + titleleft, 
		y + config->BORDER_WIDTH + config->DECORATE_BORDER_WIDTH + titleup, 
		w-2*(config->DECORATE_BORDER_WIDTH) - ((titleright || titleleft)?titleh:0), 
		h-2*(config->DECORATE_BORDER_WIDTH) - ((titleup || titledown)?titleh:0));

}

void WindowManager::MonocleMode(int x, int y, int w, int h, Desktop *d) {
	for (auto c = d->clients.begin(); c != d->clients.end(); c++) {
		if (!IsFloatOrFullscreen(*c)) {
			if (!(*c)->isHide) {
			if (config->SHOW_DECORATE) {
				XMoveResizeWindow(display, (*c)->decorate, x + config->USELESSGAP, 
					y + config->USELESSGAP, 
					w - 2*config->USELESSGAP, 
					h - 2*config->USELESSGAP);
			}
			XMoveResizeWindow(display, (*c)->win, x + config->USELESSGAP, 
				y + config->USELESSGAP, 
				w - 2*config->USELESSGAP, 
				h - 2*config->USELESSGAP);
			}
		}
	}
}

void WindowManager::PrevDesktop(void) {
	Argument arg;
	arg.i = monitors[monitorId]->desktopPrevId;
	ChangeDesktop(&arg);
}

void WindowManager::MoveLocal(Window win, int x, int y) {
	int titleup = config->SHOW_TITLE && config->TITLE_POSITION == TITLE_UP ? config->TITLE_HEIGHT:0;
	int titleleft = config->SHOW_TITLE && config->TITLE_POSITION == TITLE_LEFT ? config->TITLE_HEIGHT:0;
	XMoveWindow(display, win, 
		x + config->BORDER_WIDTH + config->DECORATE_BORDER_WIDTH + titleleft, 
		y + config->BORDER_WIDTH + config->DECORATE_BORDER_WIDTH + titleup);
}
void WindowManager::ResizeLocal(Window win, int w, int h) {
	int titleup = (config->SHOW_TITLE && config->TITLE_POSITION == TITLE_UP) ? config->TITLE_HEIGHT:0;
	int titledown = (config->SHOW_TITLE && config->TITLE_POSITION == TITLE_DOWN) ? config->TITLE_HEIGHT:0;
	int titleleft = (config->SHOW_TITLE && config->TITLE_POSITION == TITLE_LEFT) ? config->TITLE_HEIGHT:0;
	int titleright = (config->SHOW_TITLE && config->TITLE_POSITION == TITLE_RIGHT) ? config->TITLE_HEIGHT:0;
	int titleh = config->SHOW_TITLE ? config->TITLE_HEIGHT:0;
	XResizeWindow(display, win, 
		w-2*(config->DECORATE_BORDER_WIDTH) - ((titleright!=0 || titleleft!=0)?titleh:0), 
		h-2*(config->DECORATE_BORDER_WIDTH) - ((titleup!=0 || titledown!=0)?titleh:0));
}


void WindowManager::mapRequest(XEvent *e) {
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;
	Window w = e->xmaprequest.window;
	XWindowAttributes wa = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	if (WinToClient(w, &c, &d, &m) || (XGetWindowAttributes(display, w, &wa) && wa.override_redirect)) {
		return;
	}
	XClassHint ch = {0, 0};
	bool follow = false;
	bool floating = false;
	int newmon = monitorId;
	int newdsk = monitors[monitorId]->desktopCurId;

	if (XGetClassHint(display, w, &ch)) {
		for (unsigned int i = 0; i < config->rules.size(); i++) {
			if (strstr(ch.res_class, config->rules[i].appClass) || strstr(ch.res_name, config->rules[i].appClass)) {
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
	m = monitors[newmon];
	d = m->desktops[newdsk];
	
	c = AddWindow(w, d);
	if (c == nullptr) {
		return;
	}
	c->isFull = false;
	c->isTrans = XGetTransientForHint(display, c->win, &w);
	if ((c->isFloat = (floating || d->mode == FLOAT)) && !c->isTrans) {
		auto win = config->SHOW_DECORATE ? c->decorate : c->win;
		XMoveWindow(display, win, m->x + (m->w - wa.width)/2, m->y + (m->h - wa.height)/2);

		if (config->SHOW_DECORATE && c->isDecorated) {
			MoveLocal(c->win, m->x + (m->w - wa.width)/2, 
				m->y + (m->h - wa.height)/2);
		}
	}
	int i;
	unsigned long l;
	unsigned char *state = nullptr;
	Atom a;
	if (XGetWindowProperty(display, c->win, netatoms[NET_WM_STATE], 0L, sizeof a,
		false, XA_ATOM, &a, &i, &l, &l, &state) == Success && state) {
		FullscreenMode(c, d, m, (*(Atom *)state == netatoms[NET_FULLSCREEN]));
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
			Tile(d, m);
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
		ChangeMonitor(&arg1);
		ChangeDesktop(&arg2);
	}
	//if (!c->isIgnore)
	Focus(c, d, m);

	if (!follow) {
		DesktopInfo();
	}
}

void WindowManager::MouseMotion(const Argument *Argument) {
	Monitor *m = monitors[monitorId];
	Desktop *d = m->desktops[m->desktopCurId];
	XWindowAttributes wa;
	XEvent ev;
	if (!d->GetCur() || !XGetWindowAttributes(display, d->GetCur()->win, &wa)) {
		return;
	}
	if (!d->GetCur()->isFloat && !d->GetCur()->isTrans && !config->AUTOFLOATING) {
		return;
	}
	if (Argument->i == RESIZE) {
		XWarpPointer(display, d->GetCur()->win, d->GetCur()->win, 0, 0, 0, 0, --wa.width, --wa.height);
	}
	int rx, ry, c, xw, yh; 
	unsigned int v; 
	Window w;
	if (!XQueryPointer(display, rootWin, &w, &w, &rx, &ry, &c, &c, &v) || w != d->GetCur()->win) {
		return;
	}
	if (XGrabPointer(display, rootWin, false, ButtonMask()|PointerMotionMask, GrabModeAsync,
		GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
			return;
	}
	if (!d->GetCur()->isFloat && !d->GetCur()->isTrans) {
		d->GetCur()->isFloat = true;
		Tile(d, m);
		Focus(d->GetCur(), d, m);
	}
	if (config->SHOW_DECORATE) {
		XRaiseWindow(display, d->GetCur()->decorate);
	}
	XRaiseWindow(display, d->GetCur()->win);
	do {
		XMaskEvent(display, ButtonPressMask|ButtonReleaseMask |
			PointerMotionMask | SubstructureRedirectMask, &ev);
		if (ev.type == MotionNotify) {
			xw = (Argument->i == MOVE ? wa.x:wa.width)  + ev.xmotion.x - rx;
			yh = (Argument->i == MOVE ? wa.y:wa.height) + ev.xmotion.y - ry;
			if (Argument->i == RESIZE) {
				auto win = config->SHOW_DECORATE ? d->GetCur()->decorate : d->GetCur()->win;
				if (config->SHOW_DECORATE && d->GetCur()->isDecorated) {
					ResizeLocal(d->GetCur()->win, (xw > config->MINWSZ ? xw:wa.width), 
						(yh > config->MINWSZ ? yh:wa.height));
				}
				XResizeWindow(display, win,
					xw > config->MINWSZ ? xw:wa.width, yh > config->MINWSZ ? yh:wa.height);
			} else if (Argument->i == MOVE) {
				auto win = config->SHOW_DECORATE ? d->GetCur()->decorate : d->GetCur()->win;
				if (config->SHOW_DECORATE && d->GetCur()->isDecorated) {
					MoveLocal(d->GetCur()->win, xw, yh);
				}
				XMoveWindow(display, win, xw, yh);
				
			}
		} else if (ev.type == ConfigureRequest || ev.type == MapRequest) {
			(*this.*events[ev.type])(&ev);
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(display, CurrentTime);
}

void WindowManager::MoveDown() {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId];
	if (!d->GetCur() || d->clients.size() <= 1) {
		return;
	}
	
	auto _id1 = d->GetCur()->id;
	auto _id2 = (_id1 + 1 < d->clients.size()) ? _id1 + 1 : 0;
	
	d->clients[_id1]->id = _id2;
	d->clients[_id2]->id = _id1;

	auto elem = d->clients[_id1];
	d->clients[_id1] = d->clients[_id2];
	d->clients[_id2] = elem;
	
	d->curClientId = _id2;
	d->prevClientId = _id2 > 0 ? _id2-1 : d->clients.size()-1;

	if (!d->GetCur()->isFloat && !d->GetCur()->isTrans) {
		Tile(d, monitors[monitorId]);
	}
}

void WindowManager::MoveUp(void) {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId];
	if (!d->GetCur() || d->clients.size() <= 1) {
		return;
	}
		
	auto _id1 = d->GetCur()->id;
	auto _id2 = (_id1 - 1 >= 0) ? _id1 - 1 : d->clients.size()-1;
	
	d->clients[_id1]->id = _id2;
	d->clients[_id2]->id = _id1;

	auto elem = d->clients[_id1];
	d->clients[_id1] = d->clients[_id2];
	d->clients[_id2] = elem;

	d->curClientId = _id2;
	d->prevClientId = _id2 > 0 ? _id2-1 : d->clients.size()-1;
	
	if (!d->GetCur()->isFloat && !d->GetCur()->isTrans) {
		Tile(d, monitors[monitorId]);
	}
}

void WindowManager::ToggleFloatClient() {
	Monitor *m = monitors[monitorId];
	Desktop *d = m->desktops[m->desktopCurId];
	XWindowAttributes wa;
	if (!d->GetCur()) {
		return;
	}
	auto win = config->SHOW_DECORATE ? d->GetCur()->decorate : d->GetCur()->win;
	if (!XGetWindowAttributes(display, win, &wa)) {
		return;
	}
	d->GetCur()->isFloat = !d->GetCur()->isFloat;
	Tile(d, m);
	Focus(d->GetCur(), d, m);
	if (d->GetCur()->isFloat) {
		XRaiseWindow(display, win);
		//m->w/4, m->h/4, m->w/2, m->h/2
		XMoveResizeWindow(display, win, (m->w-wa.width)/2, (m->h-wa.height)/2,
			              wa.width, wa.height);
		if (config->SHOW_DECORATE && d->GetCur()->isDecorated) {
			XRaiseWindow(display, d->GetCur()->win);
			MoveResizeLocal(d->GetCur()->win, (m->w-wa.width)/2, (m->h-wa.height)/2,
			                wa.width, wa.height);
		}

	}
}

void WindowManager::FloatMode(int x, int y, int w, int h, Desktop *d) {
	for (auto c = d->clients.begin(); c != d->clients.end(); c++) {
		(*c)->isFloat = true;
	}
}

void WindowManager::ToggleFullscreenClient() {
	Monitor *m = monitors[monitorId];
	Desktop *d = m->desktops[m->desktopCurId];
	if (!d->GetCur()) {
		return;
	}
	FullscreenMode(d->GetCur(), d, m, !d->GetCur()->isFull);
}

void WindowManager::MoveResize(const Argument *Argument) {
	Monitor *m = monitors[monitorId];
	Desktop *d = m->desktops[m->desktopCurId];
	XWindowAttributes wa;
	if (!d->GetCur() || !XGetWindowAttributes(display, d->GetCur()->win, &wa)) {
		return;
	}
	if (!d->GetCur()->isFloat && !d->GetCur()->isTrans && !config->AUTOFLOATING) {
		return;
	}
	if (!d->GetCur()->isFloat && !d->GetCur()->isTrans) {
		d->GetCur()->isFloat = true;
		Tile(d, m);
		Focus(d->GetCur(), d, m);
	}

	auto win = config->SHOW_DECORATE ? d->GetCur()->decorate : d->GetCur()->win;
	XRaiseWindow(display, win);
	XMoveResizeWindow(display, win, wa.x + Argument->intArr[0], wa.y + Argument->intArr[1],
		wa.width + Argument->intArr[2], wa.height + Argument->intArr[3]);
	if (config->SHOW_DECORATE && d->GetCur()->isDecorated) {
		XRaiseWindow(display, d->GetCur()->win);
		MoveResizeLocal(d->GetCur()->win, wa.x + Argument->intArr[0], wa.y + Argument->intArr[1],
			wa.width + Argument->intArr[2], wa.height + Argument->intArr[3]);
	}
}

void WindowManager::NextWin() {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId];
	if (d->GetCur() && d->clients.size() > 1) {
		auto c = (d->GetCur()->id + 1 < d->clients.size()) ? d->clients[d->GetCur()->id + 1] : d->clients[0];
		if (c->isHide) {
			HideClient(c, monitors[monitorId]);
		}
		Focus(c, d, monitors[monitorId]);
	}
}

void WindowManager::propertyNotify(XEvent *e) {
	Monitor *m = nullptr;
	Desktop *d = nullptr;
	Client *c = nullptr;

	if (!WinToClient(e->xproperty.window, &c, &d, &m)) {
		return;
	}

	if (e->xproperty.atom == XA_WM_HINTS) {
		XWMHints *wmh = XGetWMHints(display, c->win);
		Desktop *cd = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId];
		c->isUrgn = (c != cd->GetCur() && wmh && (wmh->flags & XUrgencyHint));
		if (wmh) {
			XFree(wmh);
		}
	} else if (e->xproperty.atom == XA_WM_NAME) {
		XTextProperty name;
		XGetTextProperty(display, c->win, &name, XA_WM_NAME);
		if (name.nitems && name.encoding == XA_STRING) {
			c->title = (char *)name.value;
			Focus(c, d, m);
		}
		if (name.value) {
			XFree(name.value);
		}
	}
	
	DesktopInfo();
}

void WindowManager::Sigchld(int sig) {
	if (signal(SIGCHLD, Sigchld) != SIG_ERR) {
		while(0 < waitpid(-1, NULL, WNOHANG));
	} else {
		Logger::Err("Cannot install SIGCHLD handler");
		//exit(-1);
	}
}


void WindowManager::Init() {

	WindowManager::Sigchld(0);
	const int screen = DefaultScreen(display);
	rootWin = RootWindow(display, screen);

	/* initialize monitors and desktops */
	XineramaScreenInfo *info = XineramaQueryScreens(display, &monitorCount);
	if (!monitorCount || !info){
		Logger::Err("Xinerama is not active");
		exit(-1);
	}

	monitors.resize(monitorCount);

	for (int m = 0; m < monitorCount; m++) {
		Logger::Debug("Desktop Size %i\n", config->DESKTOPS);
		Monitor *monitor = new Monitor(config->DESKTOPS);
		monitor->x = info[m].x_org;
		monitor->y = info[m].y_org;
		monitor->w = info[m].width;
		monitor->h = info[m].height; 
		monitors[m] = monitor;
		for (unsigned int d = 0; d < config->DESKTOPS; d++) {
			Desktop *desk = new Desktop();
			desk->nm = config->NMASTER;
			auto mode = config->initLayout[d] != -1 ? config->initLayout[d] : config->DEFAULT_MODE;
			desk->mode = mode;
			desk->isBar = config->SHOW_PANEL;
			monitors[m]->desktops[d] = desk;
		}
	}
	XFree(info);

	winFocus = GetColor(config->FOCUS_COLOR, screen);
	winUnfocus = GetColor(config->UNFOCUS_COLOR, screen);
	winInfocus = GetColor(config->INFOCUS_COLOR, screen);

	decorateWinFocus = GetColor(config->DECORATE_FOCUS_COLOR, screen);
	decorateWinUnfocus = GetColor(config->DECORATE_UNFOCUS_COLOR, screen);
	decorateWinInfocus = GetColor(config->DECORATE_INFOCUS_COLOR, screen);

	titleTextColor = GetColor(config->TITLE_TEXT_COLOR, screen);

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
	wmatoms[WM_PROTOCOLS]     = XInternAtom(display, "WM_PROTOCOLS",             false);
	wmatoms[WM_DELETE_WINDOW] = XInternAtom(display, "WM_DELETE_WINDOW",         false);
	netatoms[NET_SUPPORTED]   = XInternAtom(display, "_NET_SUPPORTED",           false);
	netatoms[NET_WM_STATE]    = XInternAtom(display, "_NET_WM_STATE",            false);
	netatoms[NET_ACTIVE]      = XInternAtom(display, "_NET_ACTIVE_WINDOW",       false);
	netatoms[NET_FULLSCREEN]  = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", false);

	XChangeProperty(display, rootWin, netatoms[NET_SUPPORTED], XA_ATOM, 32,
		            PropModeReplace, (unsigned char *)netatoms, NET_COUNT);


	XSetErrorHandler(WMDetected);
	XSelectInput(display, rootWin, RootMask());
	XSync(display, False);
	XSetErrorHandler(XError);
	XSync(display, False);

	GrabKeys();
	if (this->config->DEFAULT_DESKTOP >= 0 && this->config->DEFAULT_DESKTOP < this->config->DESKTOPS) {
		Argument arg;
		arg.i = this->config->DEFAULT_DESKTOP;
		ChangeDesktop(&arg);
	}
	if (this->config->DEFAULT_MONITOR >= 0 && this->config->DEFAULT_MONITOR < monitorCount) {
		Argument arg;
		arg.i = this->config->DEFAULT_MONITOR;
		ChangeMonitor(&arg);
	}

	for (auto i = 0; i < config->autostart.size(); ++i) {
		RunCmd(&config->autostart[i]);
	}
}

template <std::size_t N>
int execvp(const char* file, const char* const (&argv)[N]) {
	assert((N > 0) && (argv[N - 1] == nullptr));
	return execvp(file, const_cast<char* const*>(argv));
}

void WindowManager::RunCmd(const Argument *arg) {
	if (fork()) {
		return;
	}
	if (display) {
		close(ConnectionNumber(display));
	}
	setsid();
	auto sz = arg->com.size();
	char * a[sz];
	for(unsigned int i = 0; i < sz; i++) {
		a[i] = arg->com[i];
	}
	execvp(arg->com[0], a);
}

void WindowManager::SwapMaster() {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId];
	if (!d->GetCur() || d->clients.size() <= 1) {
		return;
	}
	auto _id1 = d->GetCur()->id;
	auto _id2 = 0;
	
	d->clients[_id1]->id = _id2;
	d->clients[_id2]->id = _id1;

	auto elem = d->clients[_id1];
	d->clients[_id1] = d->clients[_id2];
	d->clients[_id2] = elem;
	d->prevClientId = d->clients.size()-1;
	d->curClientId = 0;
	Focus(d->GetHead(), d, monitors[monitorId]);
	
	if (!d->GetCur()->isFloat && !d->GetCur()->isTrans) {
		Tile(d, monitors[monitorId]);
	}
}

void WindowManager::SwitchMode(const Argument *Argument) {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId];
	if (d->mode != Argument->i) {
		d->mode = Argument->i;
	}
	for (auto i = 0; i < d->clients.size(); i++) {
		d->clients[i]->isHide = false;
	}
	if (d->mode != FLOAT) {
		for (auto i = 0; i < d->clients.size(); i++) {
			d->clients[i]->isFloat = d->clients[i]->isFull = false;
		}
	}
	if (d->clients.size() > 0) {
		Tile(d, monitors[monitorId]);
		Focus(d->GetCur() != nullptr ? d->GetCur() : d->GetHead(), d, monitors[monitorId]);
	}
	DesktopInfo();
}

void WindowManager::Tile(Desktop *d, Monitor *m) {
	if (d->clients.size() == 0) {
		return;
	}
	(*this.*layout[d->clients.size() > 0 ? d->mode : MONOCLE])(m->x + (d->isBar ? config->PANEL_HEIGHT_VERTICAL_LEFT : 0), 
		                                                       m->y + (config->SHOW_PANEL && d->isBar ? config->PANEL_HEIGHT_HORIZONTAL_UP : 0),
		                                                       m->w - (d->isBar ? (config->PANEL_HEIGHT_VERTICAL_LEFT+config->PANEL_HEIGHT_VERTICAL_RIGHT) : 0),
		                                                       m->h - (d->isBar ? (config->PANEL_HEIGHT_HORIZONTAL_UP+config->PANEL_HEIGHT_HORIZONTAL_DOWN) : 0), d);
}

void WindowManager::TogglePanel() {
	Monitor *m = monitors[monitorId];
	m->desktops[m->desktopCurId]->isBar = !m->desktops[m->desktopCurId]->isBar;
	Tile(m->desktops[m->desktopCurId], m);
}

void WindowManager::unmapNotify(XEvent *e) {
	Monitor *m = nullptr; 
	Desktop *d = nullptr; 
	Client *c = nullptr;
	if (WinToClient(e->xunmap.window, &c, &d, &m)) {
		if (c->isUnmap) {
			return;
		}
		if (config->SHOW_DECORATE && c->isDecorated) {
			DecorationsDestroy(c);
		}
		RemoveClient(c, d, m);
	}
}

bool WindowManager::WinToClient(Window w, Client **c, Desktop **d, Monitor **m) {
	for (int cm = 0; cm < monitorCount; cm++) {
		for (int cd = 0; cd < config->DESKTOPS; cd++) {
			*m = monitors[cm];
			*d = (*m)->desktops[cd];
			for (int i = 0; i < (*d)->clients.size(); i++) {
				*c = (*d)->clients[i];
				if (*c && (*c)->win == w) {
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

void WindowManager::PrevWin() {
	Desktop *d = monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId];
	if (d->GetCur() && d->clients.size() > 1) {
		auto c = (d->GetCur()->id - 1 >= 0) ? d->clients[d->GetCur()->id - 1] : 
			d->clients[d->clients.size()-1];
		if (c->isHide) {
			HideClient(c, monitors[monitorId]);
		}
		Focus(c, d, monitors[monitorId]);
	}
}

void WindowManager::Quit(const Argument *arg) {
	retval = arg->i;
	isRunning = false;
}

void WindowManager::RemoveClient(Client *c, Desktop *d, Monitor *m) {
	if (!c || c->id >= d->clients.size()) {
		return;
	}
	auto _cur = d->GetCur();
	auto id = c->id;
	d->clients.erase(d->clients.begin()+c->id);
	if(d->clients.size() <= 0) {
		d->curClientId  = d->prevClientId = -1;
	} else {
		d->curClientId = id-1 >= 0 ? id-1 : 0;
		d->curClientId = d->curClientId >= d->clients.size() ? 0 : d->prevClientId;
		d->prevClientId = d->curClientId-1 >= 0 ? d->curClientId-1 : 0;
		if (d->prevClientId == d->curClientId) {
			d->prevClientId = d->clients.size()-1;
		}
	}
	for(auto i = 0; i < d->clients.size(); ++i) {
		d->clients[i]->id = i;
	}
	if (c == _cur || (d->GetHead() && d->clients.size() > 1)) {
		Focus(d->GetHead(), d, m);
	}
	//if (!(c->isFloat || c->isTrans) || (d->GetHead() && d->clients.size() > 1)) {
	Tile(d, m);
	//}
	delete c;
	DesktopInfo();
}


void WindowManager::ResizeMaster(const Argument *Argument) {
	Monitor *m = monitors[monitorId];
	Desktop *d = m->desktops[m->desktopCurId];
	int msz = (d->mode == H_STACK_UP || d->mode == H_STACK_DOWN ? m->h : m->w) * config->MASTER_SIZE + (d->masterSize += Argument->i);
	if (msz >= config->MINWSZ && (d->mode == H_STACK_UP || d->mode == H_STACK_DOWN ? m->h : m->w) - msz >= config->MINWSZ) {
		Tile(d, m);
	} else {
		d->masterSize -= Argument->i;
	}
}

void WindowManager::ResizeStack(const Argument *Argument) {
	monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId]->firstStackSize += Argument->i;
	Tile(monitors[monitorId]->desktops[monitors[monitorId]->desktopCurId], monitors[monitorId]);
}

void WindowManager::NextDesktop(const Argument *arg) {
	Argument _arg;
	_arg.i = (config->DESKTOPS + monitors[monitorId]->desktopCurId + arg->i) % config->DESKTOPS;
	ChangeDesktop(&_arg);
}

void WindowManager::NextFilledDesktop(const Argument *arg) {
	Monitor *m = monitors[monitorId];
	int n = arg->i;
	while (n < config->DESKTOPS && !m->desktops[(config->DESKTOPS + m->desktopCurId + n) % config->DESKTOPS]->GetHead()) {
		(n += arg->i);
	}
	Argument _arg;
	_arg.i = (config->DESKTOPS + m->desktopCurId + n) % config->DESKTOPS;
	ChangeDesktop(&_arg);
}


void WindowManager::Run() {
	XEvent ev;
	while(isRunning && !XNextEvent(display, &ev)) {
		if (events[ev.type]) {
			(*this.*events[ev.type])(&ev);
		}
	}
}

void WindowManager::FullscreenMode(Client *c, Desktop *d, Monitor *m, bool fullscrn) {
	if (fullscrn != c->isFull) {
		XChangeProperty(display, c->win,
			netatoms[NET_WM_STATE], XA_ATOM, 32, PropModeReplace, (unsigned char*)
			((c->isFull = fullscrn) ? &netatoms[NET_FULLSCREEN]:0), fullscrn);
		if (config->SHOW_DECORATE) {
			XChangeProperty(display, c->decorate,
				netatoms[NET_WM_STATE], XA_ATOM, 32, PropModeReplace, (unsigned char*)
				((c->isFull = fullscrn) ? &netatoms[NET_FULLSCREEN]:0), fullscrn);
		}
	}
	if (fullscrn) {
		if (config->SHOW_DECORATE) {
			XMoveResizeWindow(display, c->decorate, m->x, m->y, m->w, m->h);
		}
		XMoveResizeWindow(display, c->win, m->x, m->y, m->w, m->h);
	} else {
		Tile(d, m);
	}
	auto win = config->SHOW_DECORATE ? c->decorate : c->win;
	XSetWindowBorderWidth(display, win, (c->isFull || d->clients.size() <= 1 ? 0 : config->BORDER_WIDTH));
}

int WindowManager::WMDetected(Display* display, XErrorEvent* e) {
	Logger::Err("Another WM already running");
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
	//Logger::Err(" request: %i, code %i", e->request_code, e->error_code);
	//Logger::Log(" request: " +std::to_string(e->request_code)+ ", code " + std::to_string(e->error_code));
}

void WindowManager::StackMode(int x, int y, int w, int h, Desktop *d) {
	Client *c = nullptr;
	Client *t = nullptr;
	int stackType = (int)d->mode;
	int p = 0;
	int z = (stackType == H_STACK_UP || stackType == H_STACK_DOWN ? w : h); //clientSize
	int ma = (stackType == H_STACK_UP || stackType == H_STACK_DOWN ? h : w) * config->MASTER_SIZE + d->masterSize;
	int nm = d->nm;

	int stackSize = 0;
	//get master
	int cid = 0;
	for (auto _c = d->clients.begin(); _c != d->clients.end(); _c++) {
		if (!IsFloatOrFullscreen(*_c)) {
			if (c) {
				stackSize++; //stack size
			} else {
				c = *_c; //master win
				cid = c->id;
			}
		}
	}

	if (c && !stackSize && !c->isHide) {
		auto win = config->SHOW_DECORATE ? c->decorate : c->win;
		if (!c->isHide) {
		XMoveResizeWindow(display, win, x + config->USELESSGAP, y + config->USELESSGAP, 
			w - 2*(config->BORDER_WIDTH+config->USELESSGAP), 
			h - 2*(config->BORDER_WIDTH+config->USELESSGAP));
		if (config->SHOW_DECORATE && c->decorate) {
			MoveResizeLocal(c->win, x + config->USELESSGAP, y + config->USELESSGAP, 
				w - 2*(config->BORDER_WIDTH+config->USELESSGAP), 
				h - 2*(config->BORDER_WIDTH+config->USELESSGAP));
		}
		}
	}
	if (!c || !stackSize) {
		return;
	} else if (stackSize - nm <= 0) { 
		nm = stackSize;
	} else {
		stackSize -= nm-1;
		p = (z - d->firstStackSize)%stackSize + d->firstStackSize;
		z = (z - d->firstStackSize)/stackSize; 
	}
	for (int i = 0; i < nm; i++) {
	if (c->isHide) {
		for (auto i = cid+1; i < d->clients.size(); i++) {
			if (!IsFloatOrFullscreen(d->clients[i])) {
				c = d->clients[i];
				cid = c->id;
				break;
			}
		}
		continue;
	}
	auto win = config->SHOW_DECORATE ? c->decorate : c->win;
	if (stackType == H_STACK_DOWN) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			MoveResizeLocal(c->win, x+config->USELESSGAP + i * (w-config->USELESSGAP)/nm, 
				y+config->USELESSGAP, 
				(w-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP, 
				ma - 2*(config->BORDER_WIDTH+config->USELESSGAP));
		}
		XMoveResizeWindow(display, win, x+config->USELESSGAP + i * (w-config->USELESSGAP)/nm, 
			y+config->USELESSGAP, 
			(w-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP, 
			ma - 2*(config->BORDER_WIDTH+config->USELESSGAP));
	} else if (stackType == V_STACK_LEFT) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			MoveResizeLocal(c->win, (w-ma+(d->isBar && config->SHOW_PANEL ? config->PANEL_HEIGHT_VERTICAL_LEFT : 0))+config->USELESSGAP, 
				y+config->USELESSGAP + i * (h-config->USELESSGAP)/nm, 
				ma - 2*(config->BORDER_WIDTH+config->USELESSGAP), 
				(h-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP);
		}
		XMoveResizeWindow(display, win, (w-ma+(d->isBar && config->SHOW_PANEL ? config->PANEL_HEIGHT_VERTICAL_LEFT : 0))+config->USELESSGAP, 
			y+config->USELESSGAP + i * (h-config->USELESSGAP)/nm, 
			ma - 2*(config->BORDER_WIDTH+config->USELESSGAP), 
			(h-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP);
	} else if (stackType == V_STACK_RIGHT) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			MoveResizeLocal(c->win, x+config->USELESSGAP, 
			y+config->USELESSGAP + i * (h-config->USELESSGAP)/nm, 
			ma - 2*(config->BORDER_WIDTH+config->USELESSGAP), 
			(h-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP);
		}
		XMoveResizeWindow(display, win, x+config->USELESSGAP, 
			y+config->USELESSGAP + i * (h-config->USELESSGAP)/nm, 
			ma - 2*(config->BORDER_WIDTH+config->USELESSGAP), 
			(h-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP);
	} else if (stackType == H_STACK_UP) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			MoveResizeLocal(c->win, x+config->USELESSGAP + i * (w-config->USELESSGAP)/nm, 
				(h-ma+(d->isBar && config->SHOW_PANEL ? config->PANEL_HEIGHT_HORIZONTAL_UP : 0))+config->USELESSGAP, 
				(w-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP, 
				ma - 2*(config->BORDER_WIDTH+config->USELESSGAP));
		}
		XMoveResizeWindow(display, win, x+config->USELESSGAP + i * (w-config->USELESSGAP)/nm, 
			(h-ma+(d->isBar && config->SHOW_PANEL ? config->PANEL_HEIGHT_HORIZONTAL_UP : 0))+config->USELESSGAP, 
			(w-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP, 
			ma - 2*(config->BORDER_WIDTH+config->USELESSGAP));
	}
	for (auto i = cid+1; i < d->clients.size(); i++) {
		if (!IsFloatOrFullscreen(d->clients[i])) {
			c = d->clients[i];
			cid = c->id;
			break;
		}
	}
	}
	/*
	for (auto i = cid+1; i < d->clients.size(); i++) {
		if (!IsFloatOrFullscreen(d->clients[i])) {
			c = d->clients[i];
			cid = c->id;
			break;
		}
	}*/
	
	int cw = (stackType == V_STACK_LEFT || stackType == V_STACK_RIGHT ? w : h) - 2*config->BORDER_WIDTH - config->USELESSGAP - ma;
	int ch = z - 2*config->BORDER_WIDTH - config->USELESSGAP;
	auto win = config->SHOW_DECORATE ? c->decorate : c->win;
	
	if (stackType == V_STACK_RIGHT) {
		x += ma;
		y += config->USELESSGAP;
		if (!c->isHide) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			MoveResizeLocal(c->win, x, y, cw, ch - config->USELESSGAP + p);
		}
		XMoveResizeWindow(display, win, x, y, cw, ch - config->USELESSGAP + p);
		}
	} else if (stackType == V_STACK_LEFT) {
		y += config->USELESSGAP;
		x += config->USELESSGAP;
		if (!c->isHide) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			MoveResizeLocal(c->win, x, y, cw, ch - config->USELESSGAP + p);
		}
		XMoveResizeWindow(display, win, x, y, cw, ch - config->USELESSGAP + p);
		}
	} else if (stackType == H_STACK_UP) {
		x += config->USELESSGAP;
		y += config->USELESSGAP;
		if (!c->isHide) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			MoveResizeLocal(c->win, x, y, ch  - config->USELESSGAP + p, cw);
		}
		XMoveResizeWindow(display, win, x, y, ch  - config->USELESSGAP + p, cw);
		}
	} else {
		y += ma;
		x += config->USELESSGAP;
		if (!c->isHide) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			MoveResizeLocal(c->win, x, y, ch - config->USELESSGAP + p, cw);
		}
		XMoveResizeWindow(display, win, x, y, ch - config->USELESSGAP + p, cw);
		}
	}
	
	stackType == V_STACK_LEFT || stackType == V_STACK_RIGHT ? (y += z+p - config->USELESSGAP) : (x += z+p - config->USELESSGAP);

	for (auto i = cid+1; i < d->clients.size(); i++) {
		if (IsFloatOrFullscreen(d->clients[i])) {
			continue;
		}
		auto _win = config->SHOW_DECORATE ? (d->clients[i])->decorate : (d->clients[i])->win;
		if (stackType == V_STACK_LEFT || stackType == V_STACK_RIGHT) { 
			if (!d->clients[i]->isHide){
			if (config->SHOW_DECORATE && (d->clients[i])->isDecorated) {
				MoveResizeLocal((d->clients[i])->win, x, y, cw, ch);
			}
			XMoveResizeWindow(display, _win, x, y, cw, ch);
			}
			y += z;
		} else {
			if (!d->clients[i]->isHide){
			if (config->SHOW_DECORATE && (d->clients[i])->isDecorated) {
				MoveResizeLocal((d->clients[i])->win, x, y, ch, cw);
			}
			XMoveResizeWindow(display, _win, x, y, ch, cw);
			}
			x += z;
		}
	}
}

void WindowManager::FibonacciMode(int x, int y, int w, int h, Desktop *d) {
	int j = -1;
	int cw = w - config->BORDER_WIDTH - config->USELESSGAP*2;
	int ch = h - config->BORDER_WIDTH - config->USELESSGAP*2;
	
	Client *n = nullptr;
	Client *c = nullptr;

	auto winCount = 0;
	for (auto i = 0; i < d->clients.size(); ++i) {
		if (!IsFloatOrFullscreen(d->clients[i])) {
			winCount++;
		}
	}
	x += config->USELESSGAP;
	y += config->USELESSGAP;
	for (auto i = 0; i < d->clients.size(); ++i) {
		c = d->clients[i];
		if (IsFloatOrFullscreen(c)) {
			continue;
		} else {
			j++;
		}
		for (auto ii = i+1; ii < d->clients.size(); ++ii) {
			n = d->clients[ii];
			if (!IsFloatOrFullscreen(n)) {
				break;
			}
		}
		if (n && winCount!=1) (j&1) ? ((ch /= 2)) : ((cw /= 2));
		if (j) (j&1) ? (x += cw + config->USELESSGAP) : (y += ch + config->USELESSGAP);
		auto win = config->SHOW_DECORATE ? c->decorate : c->win;
		if (!c->isHide){
		if (config->SHOW_DECORATE && c->isDecorated) {
			MoveResizeLocal(c->win, x, y, cw - config->BORDER_WIDTH, ch - config->BORDER_WIDTH);
		}
		XMoveResizeWindow(display, win, x, y, cw - config->BORDER_WIDTH, ch - config->BORDER_WIDTH);
		}
		if (n && winCount!=1)
		if (j&1) {
			ch-=config->USELESSGAP;
			y += config->USELESSGAP;
		} else {
			cw-=config->USELESSGAP;
			x += config->USELESSGAP;
		}
		winCount--;
	}
}
