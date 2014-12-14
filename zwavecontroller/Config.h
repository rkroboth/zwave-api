#ifndef _CONFIG_H
#define _CONFIG_H

#include "headers.h"
#include "Utils.h"

class Config {
public:
	boost::property_tree::ptree props;
	boost::property_tree::ptree defaults;

	static Config LoadConfig(std::string path_to_config_file);

	string GetConfig(string identifier);
};

#endif
