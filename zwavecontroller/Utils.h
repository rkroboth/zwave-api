#ifndef _UTILS_H
#define _UTILS_H

#include "headers.h"

class Utils {
public:
	static std::wstring StringToWString(const string& s);
	static std::string WStringToString(const wstring& s);
	static std::string trim(string s, string replaceChars);
	static std::string GetProgramExecutableDir();
	static uint64 GetTimeMilliseconds();
	static bool IsValidDirectory(string dir);
	static bool IsValidFile(string filepath);
};

#endif
