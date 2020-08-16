#include "logger.h"
#include <iostream>

using namespace SomeLogger;

#ifdef _WIN32
#else
std::map<Color, std::string> colorToFgColor {
	{ Color::Black, "30" },
	{ Color::Grey, "1;30" },
	{ Color::LightGrey, "1;37" },
	{ Color::White, "37" },
	{ Color::Blue, "34"} ,
	{ Color::Green, "32" },
	{ Color::Cyan, "36" },
	{ Color::Red, "31" },
	{ Color::Purple, "35" },
	{ Color::LightBlue, "1;34" },
	{ Color::LightGreen, "1;32" },
	{ Color::LightCyan, "1;36" },
	{ Color::LightRed, "1;31" },
	{ Color::LightPurple, "1;35" },
	{ Color::Orange, "1;33" },
	{ Color::Yellow, "33" }
};

std::map<Color, std::string> colorToBgColor {
	{ Color::Black, "40" },
	{ Color::Grey, "1;40" },
	{ Color::LightGrey, "1;47" },
	{ Color::White, "47" },
	{ Color::Blue, "44"} ,
	{ Color::Green, "42" },
	{ Color::Cyan, "46" },
	{ Color::Red, "41" },
	{ Color::Purple, "45" },
	{ Color::LightBlue, "1;44" },
	{ Color::LightGreen, "1;42" },
	{ Color::LightCyan, "1;46" },
	{ Color::LightRed, "1;41" },
	{ Color::LightPurple, "1;45" },
	{ Color::Orange, "1;43" },
	{ Color::Yellow, "43" }
};
#endif

Logger::Logger() {
	#ifdef _WIN32
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	#else
	#endif
}
char* Logger::convert(unsigned int num, int base) {
	static char Representation[]= "0123456789ABCDEF";
	static char buffer[50];
	char *ptr;
	ptr = &buffer[49];
	*ptr = '\0';
	do {
		*--ptr = Representation[num % base];
		num /= base;
	} while(num != 0);
	return(ptr);
}
void Logger::out(const String& msg) {
    #ifdef _WIN32
	SetConsoleTextAttribute(hConsole, (static_cast<int>(fgColor) + (static_cast<int>(bgColor)<<1)));
	std::cout <<levelLabel[level] << msg.asStr() << (isEndl ? "\n" : "");
	SetConsoleTextAttribute(hConsole, 0x0f);
	#else
	//FILE *f = fopen("/home/daniil/Documents/unknowwm/log", "a");
	//fprintf(f, msg.asStr().c_str());
	//fclose(f);
	std::cout << std::string("\033[") + colorToFgColor[fgColor] + std::string(";") + colorToBgColor[bgColor] +
		std::string("m") + levelLabel[level] + msg.asStr() + std::string("\033[0m\n") + (isEndl ? "\n" : "");
	#endif
}

void Logger::setLabelForLevel(LoggerLevel level, std::string label) {
	levelLabel[level] = label;
}
Logger& Logger::operator<<(const std::string& msg) {
	out(String(msg));
	return *this;
}
Logger& Logger::operator<<(const int& msg) {
	out(String(msg));
	return *this;
}
Logger& Logger::operator<<(const long& msg) {
	out(String(msg));
	return *this;
}
Logger& Logger::operator<<(const long long& msg) {
	out(String(msg));
	return *this;
}
Logger& Logger::operator<<(const unsigned& msg) {
	out(String(msg));
	return *this;
}
Logger& Logger::operator<<(const unsigned long& msg) {
	out(String(msg));
	return *this;
}
Logger& Logger::operator<<(const unsigned long long& msg) {
	out(String(msg));
	return *this;
}
Logger& Logger::operator<<(const float& msg) {
	out(String(msg));
	return *this;
}
Logger& Logger::operator<<(const double& msg) {
	out(String(msg));
	return *this;
}
Logger& Logger::operator<<(const long double& msg) {
	out(String(msg));
	return *this;
}
Logger& Logger::log(LoggerLevel level, Color fgColor, Color bgColor) {
	this->fgColor = fgColor;
	this->bgColor = bgColor;
	this->level = level;
	return *this;
}
Logger& Logger::printEndl(bool isEndl) {
    this->isEndl = isEndl;
    return *this;
}
void Logger::logFormat(char* format, ...) {
    std::string allFormat = format;
	#ifdef _WIN32
	SetConsoleTextAttribute(hConsole, (static_cast<int>(fgColor) + (static_cast<int>(bgColor)<<1)));
	#else
	puts((std::string("\033[") + colorToFgColor[fgColor] + std::string(";") + colorToBgColor[bgColor] + std::string("m")).c_str());
	#endif
	const char *traverse;
	unsigned int i;
	char *s;
	allFormat = levelLabel[level] + allFormat;
	va_list arg;
	va_start(arg, allFormat.c_str());
    bool needCheck = true;
	for (auto& traverse : allFormat) {
	    if (traverse == '\0') {
            break;
	    }
		if (traverse != '%' && needCheck) {
            putchar(traverse);
            continue;
		} else if (needCheck) {
		    needCheck = false;
            continue;
		}
        needCheck = true;
		switch (traverse) {
			case 'c' : i = va_arg(arg, int);	//Fetch char argument
						putchar(i);
						break;
			case 'd' : i = va_arg(arg, int); //Fetch Decimal/Integer argument
						if (i < 0) {
							i = -i;
							putchar('-');
						}
						puts(convert(i,10));
						break;
			case 'o': i = va_arg(arg, unsigned int); //Fetch Octal representation
						puts(convert(i, 8));
						break;
			case 's': s = va_arg(arg, char *); //Fetch string
						puts(s);
						break;
			case 'x': i = va_arg(arg, unsigned int); //Fetch Hexadecimal representation
						puts(convert(i, 16));
						break;
			case 'b': i = va_arg(arg,unsigned int); //Fetch binary representation
						puts(convert(i, 2));
						break;
		}
	}
	if (isEndl) {
        putchar('\n');
	}
	va_end(arg);
	#ifdef _WIN32
	SetConsoleTextAttribute(hConsole, 0x0f);
	#else
	puts("\033[0m\n");
	#endif
}

