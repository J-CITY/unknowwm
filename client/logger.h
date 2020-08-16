#pragma once
//#pragma one
#include <map>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else

#endif
#include<stdarg.h>

#define PRINT_LINE __LINE__
#define PRINT_LINE_STR std::to_string(__LINE__)

namespace SomeLogger {

enum class LoggerLevel {
	NONE = 0,
	ERR,
	WARNING,
	DEBUG,
	INFO
};
enum class Color {
	Black = 0,
	Blue,
	Green,
	Red,
	Grey,
	LightGrey,
	Cyan,
	Purple,
	LightBlue,
	LightGreen,
	LightCyan,
	LightRed,
	LightPurple,
	Orange,
	Yellow,
	White
};


class String {
	std::string data = "";
public:
	String() {}

	String(std::string in) {
		data = in;
	}
	String(int in) {
		data = std::to_string(in);
	}
	String(long in) {
		data = std::to_string(in);
	}
	String(long long in) {
		data = std::to_string(in);
	}
	String(unsigned in) {
		data = std::to_string(in);
	}
	String(unsigned long in) {
		data = std::to_string(in);
	}
	String(unsigned long long in) {
		data = std::to_string(in);
	}
	String(float in) {
		data = std::to_string(in);
	}
	String(double in) {
		data = std::to_string(in);
	}
	String(long double in) {
		data = std::to_string(in);
	}
	String(bool in) {
		data = in ? "true" : "false";
	}
	String(char in) {
		data = std::string(1, in);
	}
	String(unsigned char in) {
		data = std::string(1, in);
	}

	std::string asStr() const {
		return data;
	}
};


class Logger {
public:
	static Logger& Instance() {
			static Logger logger;
			return logger;
	}

	std::map<LoggerLevel, std::string> levelLabel {
		{ LoggerLevel::NONE, "" },
		{ LoggerLevel::ERR, "ERROR " },
		{ LoggerLevel::WARNING, "WARNING " },
		{ LoggerLevel::DEBUG, "DEBUG " },
		{ LoggerLevel::INFO, "INFO " }
	};

	void setLabelForLevel(LoggerLevel level, std::string label);
	Logger& operator<<(const std::string& msg);
	Logger& operator<<(const int& msg);
	Logger& operator<<(const long& msg);
	Logger& operator<<(const long long& msg);
	Logger& operator<<(const unsigned& msg) ;
	Logger& operator<<(const unsigned long& msg);
	Logger& operator<<(const unsigned long long& msg);
	Logger& operator<<(const float& msg);
	Logger& operator<<(const double& msg) ;
	Logger& operator<<(const long double& msg);
	Logger& log(LoggerLevel level=LoggerLevel::NONE,
				Color fgColor=Color::White,
				Color bgColor=Color::Black);
    Logger& printEndl(bool isEndl);

	void logFormat(char* format, ...);

	~Logger() = default;
private:
    bool isEndl = false;
	Color fgColor = Color::White;
	Color bgColor = Color::Black;
	LoggerLevel level = LoggerLevel::NONE;
	#ifdef _WIN32
	HANDLE  hConsole;
	#endif

	Logger();
	char* convert(unsigned int num, int base);

	void out(const String& msg);
};

};
