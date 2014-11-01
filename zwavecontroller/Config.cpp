#include "stdafx.h"
#include "Config.h"

string Config::GetConfig(string identifier){
	string v;

	try {
		v = this->props.get<std::string>("config." + identifier);
		if (
			identifier == "data_dir"
			|| identifier == "log_dir"
			|| identifier == "zwave_manufacturer_xml_dir"
		){
			if (v != "" && !Utils::IsValidDirectory(v)){
				v = "";
			}
		}
	}
	catch (...){
		v = "";
	}

	if (v == ""){
		try {
			v = this->defaults.get<std::string>(identifier);
		}
		catch (...){
			v = "";
		}
	}

	return v;
}

Config Config::LoadConfig(string path_to_config_file) {

	Config this_config = Config();

	bool found = false;

	// first try to get from passed-in filename (command line)
	if (!path_to_config_file.empty()){
		found = Utils::IsValidFile(path_to_config_file);
	}

	// next try to get from program executable directory
	if (!found){
		path_to_config_file = Utils::GetProgramExecutableDir();
		path_to_config_file.append("/config.xml");
		found = Utils::IsValidFile(path_to_config_file);
	}

	// if config file was found, load up the config from the file
	if (found){
		std::ifstream ifs(path_to_config_file);
		std::string config((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

		if (!config.empty()){
			// load json

			try {
				stringstream ss;
				ss << config;
				read_xml(ss, this_config.props);
			}
			catch (...){
			}

		}
	}

	this_config.defaults.put("api_port", "8081");

	srand(static_cast<unsigned int>(time(NULL)));
	int r = (rand() % 1000000 + 1);
	this_config.defaults.put("api_key", std::to_string(r));

	this_config.defaults.put("data_dir", Utils::GetProgramExecutableDir());

	this_config.defaults.put("log_dir", Utils::GetProgramExecutableDir());

	this_config.defaults.put("logfile_prefix", "zwavecontroller_");

	this_config.defaults.put("zwave_manufacturer_xml_dir", Utils::GetProgramExecutableDir() + "/openzwave/config");

	this_config.defaults.put("controller_com_port", "1");

	return this_config;
}




