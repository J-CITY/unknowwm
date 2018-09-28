#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

#include <cstdio>
#include <stdarg.h>

class Logger {
public:
	Logger() {}

	static bool isLog;
	static bool isErr;
	static bool isDebug;

	static void Log(const char *format, ...) {
		if (Logger::isLog) {
			printf("LOG: ");
			va_list vargs;
			va_start(vargs, format);
			vprintf(format, vargs);
			printf("\n");
			va_end(vargs);
			/*FILE *f = fopen("/home/daniil/WM/log", "a");
			fprintf(f, format, vargs);
			//fflush(file); 
			fclose(f);*/
		}
	}

	static void Log(std::string str) {
		Log(str.c_str());
	}

	static void Err(const char *format, ...) {
		if (Logger::isErr) {
			printf("ERR: ");
			va_list vargs;
			va_start(vargs, format);
			vprintf(format, vargs);
			printf("\n");
			va_end(vargs);
			/*FILE *f = fopen("/home/daniil/WM/log", "a");
			fprintf(f, format, vargs);
			//fflush(file); 
			fclose(f);*/
		}
	}
	
	static void Err(std::string str) {
		Err(str.c_str());
	}

	static void Debug(const char *format, ...) {
		if (Logger::isDebug) {
			printf("DEBUG: ");
			va_list vargs;
			va_start(vargs, format);
			vprintf(format, vargs);
			printf("\n");
			va_end(vargs);
			/*FILE *f = fopen("/home/daniil/WM/log", "a");
			fprintf(f, format, vargs);
			//fflush(file); 
			fclose(f);*/
		}
	}

	static void Debug(std::string str) {
		Debug(str.c_str());
	}
};

bool Logger::isLog = true;
bool Logger::isErr = true;
bool Logger::isDebug = true;

#endif