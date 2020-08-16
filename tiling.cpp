
#include "wm.h"

using namespace UW;

void WindowManager::tile(Desktop *d, Monitor *m) {
	if (d->clients.size() == 0) {
		return;
	}
	(*this.*layout[d->clients.size() > 0 ? d->mode : MONOCLE])(m->x + (d->isBar ? config->PANEL_HEIGHT_VERTICAL_LEFT : 0), 
		                                                       m->y + (config->SHOW_PANEL && d->isBar ? config->PANEL_HEIGHT_HORIZONTAL_UP : 0),
		                                                       m->w - (d->isBar ? (config->PANEL_HEIGHT_VERTICAL_LEFT+config->PANEL_HEIGHT_VERTICAL_RIGHT) : 0),
		                                                       m->h - (d->isBar ? (config->PANEL_HEIGHT_HORIZONTAL_UP+config->PANEL_HEIGHT_HORIZONTAL_DOWN) : 0), d);
}

void WindowManager::gridMode(int x, int y, int w, int h, Desktop *d) {
	int n = 0;
	int cols = 0;
	int cn = 0;
	int rn = 0;
	int i = -1;
	
	for (auto& c : d->clients) {
		if (!isFloatOrFullscreen(c.get())) {
			++n;
		}
	}

	for (cols = 0; cols <= n / 2; cols++) {
		if (cols * cols >= n) {
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
	for (auto& c : d->clients) {
		if (isFloatOrFullscreen(c.get())) {
			continue; 
		} else {
			++i;
		}
		if (i/rows + 1 > cols - n % cols) {
			rows = n/cols + 1;
		}
		auto win = config->SHOW_DECORATE ? c.get()->decorate : c.get()->win;
		if (!c.get()->isHide){
		XMoveResizeWindow(display, win, x + cn*cw + config->USELESSGAP, 
			y + rn*ch/rows + config->USELESSGAP, 
			cw - 2*config->BORDER_WIDTH-config->USELESSGAP, 
			ch/rows - 2*config->BORDER_WIDTH-config->USELESSGAP);
		
		if (config->SHOW_DECORATE && c.get()->isDecorated) {
			c->moveResizeLocal(x + cn*cw + config->USELESSGAP, 
				y + rn*ch/rows + config->USELESSGAP, 
				cw - 2*config->BORDER_WIDTH-config->USELESSGAP, 
				ch/rows - 2*config->BORDER_WIDTH-config->USELESSGAP, *config, display);
		}
		}
		if (++rn >= rows) {
			rn = 0;
			cn++;
		}
	}
}

void WindowManager::monocleMode(int x, int y, int w, int h, Desktop *d) {
	for (auto& c : d->clients) {
		if (!isFloatOrFullscreen(c.get())) {
			if (!c->isHide) {
			if (config->SHOW_DECORATE) {
				XMoveResizeWindow(display, c->decorate, x + config->USELESSGAP, 
					y + config->USELESSGAP, 
					w - 2*config->USELESSGAP, 
					h - 2*config->USELESSGAP);
			}
			XMoveResizeWindow(display, c->win, x + config->USELESSGAP, 
				y + config->USELESSGAP, 
				w - 2*config->USELESSGAP, 
				h - 2*config->USELESSGAP);
			}
		}
	}
}

void WindowManager::floatMode(int x, int y, int w, int h, Desktop *d) {
	for (auto& c : d->clients) {
		c->isFloat = true;
	}
}

void WindowManager::fullscreenMode(Client *c, Desktop *d, Monitor *m, bool fullscrn) {
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
		tile(d, m);
	}
	auto win = config->SHOW_DECORATE ? c->decorate : c->win;
	XSetWindowBorderWidth(display, win, (c->isFull || d->clients.size() <= 1 ? 0 : config->BORDER_WIDTH));
}


void WindowManager::stackMode(int x, int y, int w, int h, Desktop *d) {
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
	for (auto& _c : d->clients) {
		if (!isFloatOrFullscreen(_c.get())) {
			if (c) {
				stackSize++; //stack size
			} else {
				c = _c.get(); //master win
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
			c->moveResizeLocal(x + config->USELESSGAP, y + config->USELESSGAP, 
				w - 2*(config->BORDER_WIDTH+config->USELESSGAP), 
				h - 2*(config->BORDER_WIDTH+config->USELESSGAP), *config, display);
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
	    		if (!isFloatOrFullscreen(d->clients[i].get())) {
	    			c = d->clients[i].get();
	    			cid = c->id;
	    			break;
	    		}
	    	}
	    	continue;
	    }
	    auto win = config->SHOW_DECORATE ? c->decorate : c->win;
	    if (stackType == H_STACK_DOWN) {
	    	if (config->SHOW_DECORATE && c->isDecorated) {
	    		c->moveResizeLocal(x+config->USELESSGAP + i * (w-config->USELESSGAP)/nm, 
	    			y+config->USELESSGAP, 
	    			(w-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP, 
	    			ma - 2*(config->BORDER_WIDTH+config->USELESSGAP), *config, display);
	    	}
	    	XMoveResizeWindow(display, win, x+config->USELESSGAP + i * (w-config->USELESSGAP)/nm, 
	    		y+config->USELESSGAP, 
	    		(w-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP, 
	    		ma - 2*(config->BORDER_WIDTH+config->USELESSGAP));
	    } else if (stackType == V_STACK_LEFT) {
	    	if (config->SHOW_DECORATE && c->isDecorated) {
	    		c->moveResizeLocal((w-ma+(d->isBar && config->SHOW_PANEL ? config->PANEL_HEIGHT_VERTICAL_LEFT : 0))+config->USELESSGAP, 
	    			y+config->USELESSGAP + i * (h-config->USELESSGAP)/nm, 
	    			ma - 2*(config->BORDER_WIDTH+config->USELESSGAP), 
	    			(h-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP, *config, display);
	    	}
	    	XMoveResizeWindow(display, win, (w-ma+(d->isBar && config->SHOW_PANEL ? config->PANEL_HEIGHT_VERTICAL_LEFT : 0))+config->USELESSGAP, 
	    		y+config->USELESSGAP + i * (h-config->USELESSGAP)/nm, 
	    		ma - 2*(config->BORDER_WIDTH+config->USELESSGAP), 
	    		(h-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP);
	    } else if (stackType == V_STACK_RIGHT) {
	    	if (config->SHOW_DECORATE && c->isDecorated) {
	    		c->moveResizeLocal(x+config->USELESSGAP, 
	    		y+config->USELESSGAP + i * (h-config->USELESSGAP)/nm, 
	    		ma - 2*(config->BORDER_WIDTH+config->USELESSGAP), 
	    		(h-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP, *config, display);
	    	}
	    	XMoveResizeWindow(display, win, x+config->USELESSGAP, 
	    		y+config->USELESSGAP + i * (h-config->USELESSGAP)/nm, 
	    		ma - 2*(config->BORDER_WIDTH+config->USELESSGAP), 
	    		(h-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP);
	    } else if (stackType == H_STACK_UP) {
	    	if (config->SHOW_DECORATE && c->isDecorated) {
	    		c->moveResizeLocal(x+config->USELESSGAP + i * (w-config->USELESSGAP)/nm, 
	    			(h-ma+(d->isBar && config->SHOW_PANEL ? config->PANEL_HEIGHT_HORIZONTAL_UP : 0))+config->USELESSGAP, 
	    			(w-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP, 
	    			ma - 2*(config->BORDER_WIDTH+config->USELESSGAP), *config, display);
	    	}
	    	XMoveResizeWindow(display, win, x+config->USELESSGAP + i * (w-config->USELESSGAP)/nm, 
	    		(h-ma+(d->isBar && config->SHOW_PANEL ? config->PANEL_HEIGHT_HORIZONTAL_UP : 0))+config->USELESSGAP, 
	    		(w-config->USELESSGAP)/nm - 2*config->BORDER_WIDTH-config->USELESSGAP, 
	    		ma - 2*(config->BORDER_WIDTH+config->USELESSGAP));
	    }
	    for (auto i = cid+1; i < d->clients.size(); i++) {
	    	if (!isFloatOrFullscreen(d->clients[i].get())) {
	    		c = d->clients[i].get();
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
			c->moveResizeLocal(x, y, cw, ch - config->USELESSGAP + p, *config, display);
		}
		XMoveResizeWindow(display, win, x, y, cw, ch - config->USELESSGAP + p);
		}
	} else if (stackType == V_STACK_LEFT) {
		y += config->USELESSGAP;
		x += config->USELESSGAP;
		if (!c->isHide) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			c->moveResizeLocal(x, y, cw, ch - config->USELESSGAP + p, *config, display);
		}
		XMoveResizeWindow(display, win, x, y, cw, ch - config->USELESSGAP + p);
		}
	} else if (stackType == H_STACK_UP) {
		x += config->USELESSGAP;
		y += config->USELESSGAP;
		if (!c->isHide) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			c->moveResizeLocal(x, y, ch  - config->USELESSGAP + p, cw, *config, display);
		}
		XMoveResizeWindow(display, win, x, y, ch  - config->USELESSGAP + p, cw);
		}
	} else {
		y += ma;
		x += config->USELESSGAP;
		if (!c->isHide) {
		if (config->SHOW_DECORATE && c->isDecorated) {
			c->moveResizeLocal(x, y, ch - config->USELESSGAP + p, cw, *config, display);
		}
		XMoveResizeWindow(display, win, x, y, ch - config->USELESSGAP + p, cw);
		}
	}
	
	stackType == V_STACK_LEFT || stackType == V_STACK_RIGHT ? (y += z+p - config->USELESSGAP) : (x += z+p - config->USELESSGAP);

	for (auto i = cid+1; i < d->clients.size(); i++) {
		if (isFloatOrFullscreen(d->clients[i].get())) {
			continue;
		}
		auto _win = config->SHOW_DECORATE ? (d->clients[i])->decorate : (d->clients[i])->win;
		if (stackType == V_STACK_LEFT || stackType == V_STACK_RIGHT) { 
			if (!d->clients[i]->isHide){
			if (config->SHOW_DECORATE && (d->clients[i])->isDecorated) {
				d->clients[i]->moveResizeLocal(x, y, cw, ch, *config, display);
			}
			XMoveResizeWindow(display, _win, x, y, cw, ch);
			}
			y += z;
		} else {
			if (!d->clients[i]->isHide){
			if (config->SHOW_DECORATE && (d->clients[i])->isDecorated) {
				d->clients[i]->moveResizeLocal(x, y, ch, cw, *config, display);
			}
			XMoveResizeWindow(display, _win, x, y, ch, cw);
			}
			x += z;
		}
	}
}

void WindowManager::fibonacciMode(int x, int y, int w, int h, Desktop *d) {
	int j = -1;
	int cw = w - config->BORDER_WIDTH - config->USELESSGAP*2;
	int ch = h - config->BORDER_WIDTH - config->USELESSGAP*2;
	
	Client *n = nullptr;
	Client *c = nullptr;

	auto winCount = 0;
	for (auto i = 0; i < d->clients.size(); ++i) {
		if (!isFloatOrFullscreen(d->clients[i].get())) {
			winCount++;
		}
	}
	x += config->USELESSGAP;
	y += config->USELESSGAP;
	for (auto i = 0; i < d->clients.size(); ++i) {
		c = d->clients[i].get();
		if (isFloatOrFullscreen(c)) {
			continue;
		} else {
			j++;
		}
		for (auto ii = i+1; ii < d->clients.size(); ++ii) {
			n = d->clients[ii].get();
			if (!isFloatOrFullscreen(n)) {
				break;
			}
		}
		if (n && winCount!=1) (j&1) ? ((ch /= 2)) : ((cw /= 2));
		if (j) (j&1) ? (x += cw + config->USELESSGAP) : (y += ch + config->USELESSGAP);
		auto win = config->SHOW_DECORATE ? c->decorate : c->win;
		if (!c->isHide){
		if (config->SHOW_DECORATE && c->isDecorated) {
			c->moveResizeLocal(x, y, cw - config->BORDER_WIDTH, ch - config->BORDER_WIDTH, *config, display);
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

void WindowManager::doubleStackVerticalMode(int x, int y, int w, int h, Desktop *d) {
	int cw = w - config->BORDER_WIDTH*2 - config->USELESSGAP*2;
	int ch = h - config->BORDER_WIDTH*2 - config->USELESSGAP*2;
	std::vector<Client*> avalibleClients;
	for (auto i = 0; i < d->clients.size(); ++i) {
		if (!isFloatOrFullscreen(d->clients[i].get())) {
			avalibleClients.push_back(d->clients[i].get());
		}
	}
	if (!avalibleClients.size()) {
		return;
	}
	x += config->USELESSGAP;
	y += config->USELESSGAP;
	if (avalibleClients.size() == 1) {
		auto win = config->SHOW_DECORATE ? avalibleClients[0]->decorate : avalibleClients[0]->win;
		if (config->SHOW_DECORATE && avalibleClients[0]->isDecorated) {
			avalibleClients[0]->moveResizeLocal(x, y, cw - config->BORDER_WIDTH, ch - config->USELESSGAP, *config, display);
		}
		XMoveResizeWindow(display, win, x, y, cw - config->BORDER_WIDTH, ch - config->USELESSGAP);
		return;
	}
	if (avalibleClients.size() == 2) {
		auto win1 = config->SHOW_DECORATE ? avalibleClients[0]->decorate : avalibleClients[0]->win;
		auto win2 = config->SHOW_DECORATE ? avalibleClients[1]->decorate : avalibleClients[1]->win;
		if (config->SHOW_DECORATE && avalibleClients[0]->isDecorated) {
			avalibleClients[0]->moveResizeLocal(x, y, (cw - config->BORDER_WIDTH - config->USELESSGAP) / 2, (ch - config->USELESSGAP), *config, display);
		}
		XMoveResizeWindow(display, win1, x, y, (cw - config->BORDER_WIDTH - config->USELESSGAP) / 2, (ch - config->USELESSGAP));

		if (config->SHOW_DECORATE && avalibleClients[1]->isDecorated) {
			avalibleClients[1]->moveResizeLocal(x+(cw - config->BORDER_WIDTH) / 2 + config->USELESSGAP, 
			y, 
			(cw - config->BORDER_WIDTH - 2*config->USELESSGAP) / 2, (ch - config->USELESSGAP), *config, display);
		}
		XMoveResizeWindow(display, win2, x+(cw - config->BORDER_WIDTH) / 2 + config->USELESSGAP, 
			y, 
			(cw - config->BORDER_WIDTH - 2*config->USELESSGAP) / 2, (ch - config->USELESSGAP));
		return;
	}
	int masterSize = d->masterSize + cw / 3 - 2 * config->BORDER_WIDTH;
	auto clientsSize = avalibleClients.size() - 1;
	int clientsRSize = 0;
	int clientsLSize = 0;
	if (clientsSize % 2 == 0) {
		clientsRSize = clientsLSize = clientsSize / 2;
	} else {
		clientsRSize = clientsSize / 2 + 1;
		clientsLSize = clientsSize / 2;
	}
	auto rStackCount = 0;
	auto lStackCount = 0;
	for (auto i = 0; i < avalibleClients.size(); ++i) {
		auto win = config->SHOW_DECORATE ? avalibleClients[i]->decorate : avalibleClients[i]->win;
		if (i == 0) {

			if (config->SHOW_DECORATE && avalibleClients[i]->isDecorated) {
				avalibleClients[i]->moveResizeLocal((cw - masterSize) / 2 + 2*config->USELESSGAP, y, 
					masterSize - config->USELESSGAP*2, (ch - config->BORDER_WIDTH), *config, display);
			}
			XMoveResizeWindow(display, win, (cw - masterSize) / 2  + 2*config->USELESSGAP, y, 
				masterSize - config->USELESSGAP*2, (ch - config->BORDER_WIDTH));
			
			continue;
		}

		auto leftOrRight = static_cast<bool>(i % 2);
		
		if (leftOrRight) { // right
			if (config->SHOW_DECORATE && avalibleClients[i]->isDecorated) {
				avalibleClients[i]->moveResizeLocal(masterSize + ((cw - masterSize) / 2) + config->USELESSGAP, 
					y+ (((ch - config->BORDER_WIDTH) / clientsRSize)) * rStackCount, 
					((cw - masterSize) / 2) - config->BORDER_WIDTH, 
					((ch - config->BORDER_WIDTH*(clientsRSize-1) - config->USELESSGAP*(clientsRSize-1)) / clientsRSize) - config->BORDER_WIDTH, *config, display);
			}
			XMoveResizeWindow(display, win, masterSize + ((cw - masterSize) / 2) + config->USELESSGAP, 
				y + (((ch - config->BORDER_WIDTH) / clientsRSize)) * rStackCount, 
				((cw - masterSize) / 2) - config->BORDER_WIDTH, 
				((ch - config->BORDER_WIDTH * (clientsRSize-1) - config->USELESSGAP * (clientsRSize-1)) / clientsRSize) - config->BORDER_WIDTH);
			rStackCount++;
		} else { // left
			if (config->SHOW_DECORATE && avalibleClients[i]->isDecorated) {
				avalibleClients[i]->moveResizeLocal(x, 
					y + (((ch - config->BORDER_WIDTH) / clientsLSize)) * lStackCount, 
					((cw - masterSize) / 2) - config->BORDER_WIDTH, 
					((ch - config->BORDER_WIDTH*(clientsLSize-1) - config->USELESSGAP*(clientsLSize-1)) / clientsLSize) - config->BORDER_WIDTH, *config, display);
			}
			XMoveResizeWindow(display, win, x, 
				y + (((ch - config->BORDER_WIDTH) / clientsLSize)) * lStackCount, 
				((cw - masterSize) / 2) - config->BORDER_WIDTH, 
				((ch - config->BORDER_WIDTH*(clientsLSize-1) - config->USELESSGAP*(clientsLSize-1)) / clientsLSize) - config->BORDER_WIDTH);
			lStackCount++;
		}
	
	}
}
