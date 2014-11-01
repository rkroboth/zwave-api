#include "stdafx.h"
#include "Logger.h"

void Logger::Initialize(string filepath, string prefix, string postfix){

//	string logfile_path = filepath + "/" + prefix + "_%Y%m%" + postfix;
	string logfile_path = filepath + "/" + prefix + "_%N" + postfix;
//	string logfile_path = filepath + "/" + prefix + postfix;

	boost::log::add_common_attributes();
	boost::log::add_file_log(
		boost::log::keywords::file_name = logfile_path,
		boost::log::keywords::auto_flush = true,
		boost::log::keywords::format = "[%TimeStamp%]: %Message%",
		boost::log::keywords::open_mode = (std::ios::out | std::ios::app),
		boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(1, 0, 0, 0)
	);

	// log levels = trace, debug, info, warning, error, fatal
	boost::log::core::get()->set_filter(
		boost::log::trivial::severity >= boost::log::trivial::info
	);

}


void Logger::LogNotice(string msg){
	BOOST_LOG_TRIVIAL(info) << msg;
}

