#include <cstdlib>
#include "wm.h"
#include <iostream>

using std::unique_ptr;
//#include "logger.h"
void argParse(int argc, char** argv) {
	if (argc == 2 && argv[1] == "-v") {
		//Logger::Log("version: %s", "0.1");
	} else if (argc != 1) {
		//Logger::Err("Wrong key");
	}
}

int main(int argc, char** argv) {
	argParse(argc, argv);
	Config *config = new Config();
	unique_ptr<WindowManager> windowManager(WindowManager::Create());
	if (!windowManager) {
		//Logger::Err("Failed to initialize window manager.");
		errx(EXIT_FAILURE, "cannot open display");
		return -1;
	}
	
	windowManager->config = config;
	windowManager->Init();
	windowManager->DesktopInfo(); 
	windowManager->Run();
	windowManager->Cleanup();

	delete config;
	return 0;
}
