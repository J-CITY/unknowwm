#include "desktop.h"
#include "config.h"

#include "logger.h"

using namespace UW;

Client *Desktop::getCur() {
	if (clients.size() == 0 || curClientId < 0 || curClientId >= clients.size()) {
		return nullptr;
	}
	return clients[curClientId].get();
}

Client *Desktop::getHead() {
	if (clients.size() == 0) {
		return nullptr;
	}
	return clients[0].get();
}

Client *Desktop::getPrev() {
	if (clients.size() == 0 || prevClientId < 0 || prevClientId >= clients.size()) {
		return nullptr;
	}
	return clients[prevClientId].get();
}

Client* Desktop::addClient(std::unique_ptr<Client> c, Config& config) {
	Client* ret = c.get();
	if (clients.size() == 0) {
		c->id = 0;
		clients.push_back(std::move(c));
		curClientId = 0;
		prevClientId = 0;
	} else if (!config.ATTACH_ASIDE) {
		clients.insert(clients.begin(), std::move(c));
		ret->id = 0;
		curClientId = 0;
		for (auto i = 0; i < clients.size(); i++) {
			clients[i]->id = i;
		}
	} else {
		c->id = clients.size();
		clients.push_back(std::move(c));
		curClientId = ret->id;
	}
	return ret;
}

void Desktop::mapAll(UW::Config& config, Display* display) {
	auto _d = getCur();
	if (_d != nullptr){
		map(_d, config, display);
	}
	for (auto& c : clients) {
		c->isUnmap = false;
		map(c.get(), config, display);
	}
}

void Desktop::map(Client* c, UW::Config& config, Display* display) {
	if (config.SHOW_DECORATE && c->isDecorated) {
		XMapWindow(display, c->decorate);
	}
	XMapWindow(display, c->win);
}

void Desktop::unmapAll(UW::Config& config, Display* display) {
	for (auto& c : clients) {
		c->isUnmap = true;
		if (c.get() != getCur()) {
			unmap(c.get(), config, display);
		}
	}
	if (getCur() != nullptr) {
		unmap(getCur(), config, display);
	}
}

void Desktop::unmap(Client* c, UW::Config& config, Display* display) {
	if (config.SHOW_DECORATE && c->isDecorated) {
		XUnmapWindow(display, c->decorate);
	}
	XUnmapWindow(display, c->win);
}

void Desktop::removeCurClient(Client* c) {
	if (!c) {
		return;
	}
	auto _cur = getCur();
	auto id = c->id;
	clients.erase(clients.begin() + id);
	if(clients.size() <= 0) {
		curClientId  = prevClientId = -1;
	} else {
		curClientId = id-1 >= 0 ? id-1 : 0;
		curClientId = curClientId >= clients.size() ? 0 : prevClientId;
		prevClientId = curClientId-1 >= 0 ? curClientId-1 : 0;
		if (prevClientId == curClientId) {
			prevClientId = clients.size()-1;
		}
	}
	for(auto i = 0; i < clients.size(); ++i) {
		clients[i]->id = i;
	}
}