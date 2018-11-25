#include <cstdlib>
#include "wm.h"
#include <iostream>

using std::unique_ptr;
//#include "logger.h"
const std::string VERSION = "v0.6";


void argParse(int argc, char** argv, std::string &configPath) {
	if (argc == 2 && argv[1] == "-v") {
		std::cout << "UnknowWM: " << VERSION << "\n";
		exit(0);
	} else if (argc == 2 && argv[1] == "-h") {
		//Logger::Log("version: %s", "0.1");
	} else if (argc == 3 && argv[1] == "-c") {
		configPath = argv[2];
	} else if (argc != 1) {
		//Logger::Err("Wrong key");
		exit(-1);
	}
}

int main(int argc, char** argv) {
	std::string configPath = "/home/daniil/unknowwm/config";
	argParse(argc, argv, configPath);
	Config *config = new Config(configPath);
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
