#include <cstdlib>
#include <iostream>
#include "wm.h"

const std::string VERSION = "v0.7";
void argParse(int argc, char** argv, std::string &configPath) {
	if (argc == 2 && argv[1] == "-v") {
		SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::INFO) << "UNKNOWN Version: " << VERSION << "\n";
		exit(0);
	} else if (argc == 2 && argv[1] == "-h") {		
		SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::INFO) << "Version: " << VERSION << "\n";
	} else if (argc == 3 && argv[1] == "-c") {
		configPath = argv[2];
	} else if (argc != 1) {
		SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::ERR) << "Wrong key\n";
		exit(-1);
	}
}

std::string configPath = "/home/d/unknowwm/config";

int main(int argc, char** argv) {
	argParse(argc, argv, configPath);

	auto config = std::make_unique<UW::Config>(configPath);
	std::unique_ptr<UW::WindowManager> windowManager(UW::WindowManager::Create());
	if (!windowManager) {
		SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::ERR) << "Can not open display\n";
		return -1;
	}
	
	windowManager->config = std::move(config);
	windowManager->init();
	windowManager->desktopInfo(); 
	windowManager->run();
	windowManager->cleanup();
	return 0;
}
