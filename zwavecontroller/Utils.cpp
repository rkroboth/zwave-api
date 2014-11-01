#include "stdafx.h"
#include "Utils.h"

wstring Utils::StringToWString(const string& s)
{
	wstring temp(s.length(), L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}


string Utils::WStringToString(const wstring& s)
{
	string temp(s.length(), ' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

string Utils::trim(string s, string replaceChars){
	bool trimmed = false;

	if (strlen(s.c_str()) == 0){
		return s;
	}

	// trim trailing spaces
	size_t endpos = s.find_last_not_of(replaceChars);
	int slen = s.length();
	if (slen != endpos + 1)
	{
		s = s.substr(0, endpos + 1);
		trimmed = true;
	}

	// trim leading spaces
	size_t startpos = s.find_first_not_of(replaceChars);
	if (startpos != 0)
	{
		s = s.substr(startpos);
		trimmed = true;
	}
	if (trimmed){
		s = trim(s, replaceChars);
	}
	return s;
}

string Utils::GetProgramExecutableDir(){
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string path = buffer;
	string::size_type pos = path.find_last_of("\\/");
	path = path.substr(0, pos);
	return path;
}

uint64 Utils::GetTimeMilliseconds(){
	uint64 current_milliseconds = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	uint64 unixtime = (time(0) * 1000) + current_milliseconds;
	return unixtime;
}

bool Utils::IsValidDirectory(string dir){
	if (
		dir.empty()
		|| GetFileAttributes(dir.c_str()) == INVALID_FILE_ATTRIBUTES
		|| GetFileAttributes(dir.c_str()) != FILE_ATTRIBUTE_DIRECTORY
		){
		return false;
	}
	return true;
}

bool Utils::IsValidFile(string filepath){
	if (
		GetFileAttributes(filepath.c_str()) == INVALID_FILE_ATTRIBUTES
		|| GetFileAttributes(filepath.c_str()) == FILE_ATTRIBUTE_DIRECTORY
	){
		return false;
	}
	return true;
}

