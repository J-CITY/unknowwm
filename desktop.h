#include <vector>
#include "client.h"

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
