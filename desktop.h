#pragma once

#include <vector>
#include "client.h"

namespace UW {
class Config;

struct Desktop {
	int mode=0, masterSize=0, firstStackSize=0, nm=0;
	int layoutId = 0;
	bool isHide = false;

	std::vector<std::unique_ptr<Client>> clients;
	int curClientId = -1;
	int prevClientId = -1;
	
	bool isBar;

	Desktop() = default;

	~Desktop() {
		clients.clear();
	}

	Client *getCur();
	Client *getHead();
	Client *getPrev();
	Client* addClient(std::unique_ptr<Client> c, Config& config);
	void mapAll(UW::Config& config, Display* display);
	void map(Client* c, UW::Config& config, Display* display);
	void unmapAll(UW::Config& config, Display* display);
	void unmap(Client* c, UW::Config& config, Display* display);
	void removeCurClient(Client* c);

};
}

