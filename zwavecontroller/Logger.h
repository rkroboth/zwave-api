#ifndef _LOGGER_H
#define _LOGGER_H

#include "headers.h"

class Logger {
private:
public:
	static void Initialize(string filepath, string prefix, string postfix);
	static void LogNotice(string msg);
};

#endif
