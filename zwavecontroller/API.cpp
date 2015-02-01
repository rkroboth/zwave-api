#include "stdafx.h"
#include "API.h"

#include "Logger.h"

//	private:
CivetServer* API::civit_server;
Config API::config;

bool API::handleAll(const char * method, CivetServer *server, struct mg_connection *conn) {

	string s = "";
	struct mg_request_info * req_info = mg_get_request_info(conn);
	string url(req_info->uri);
	string remote_addr(req_info->remote_addr);

	json_spirit::Object response;

	if (!CivetServer::getParam(conn, "api_key", s) || s != API::config.GetConfig("api_key")) {
		response.push_back(json_spirit::Pair("success", false));
		response.push_back(json_spirit::Pair("error_msg", "Unauthorized"));
		mg_printf(conn, "HTTP/1.1 401 Unauthorized\r\nContent-Type: application/json\r\n\r\n");
		mg_printf(conn, write(response).c_str());
		return true;
	}

	if (ZWaveController::init_failed)
	{
		response.push_back(json_spirit::Pair("success", false));
		response.push_back(json_spirit::Pair("error_msg", "ZWave driver initialization failed, not continuing."));
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
		mg_printf(conn, write(response).c_str());
		return true;
	}

	if (url == "/api/heal_network"){
		string err_message;
		bool success = ZWaveController::HealNetwork(err_message);
		if (!success){
			response.push_back(json_spirit::Pair("success", false));
			response.push_back(json_spirit::Pair("error_msg", err_message));
		}
		else {
			response.push_back(json_spirit::Pair("success", true));
		}
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
		mg_printf(conn, write(response).c_str());
		return true;
	}

	if (!ZWaveController::initial_node_queries_complete)
	{
		response.push_back(json_spirit::Pair("success", false));
		response.push_back(json_spirit::Pair("error_msg", "Not yet completed initializing the node values, please try again in a minute"));
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
		mg_printf(conn, write(response).c_str());
		return true;
	}

	if (url == "/api/get_values"){

		string s_since;
		uint64 since = 0;
		string s_node_id;
		uint64 node_id = 0;
		if (CivetServer::getParam(conn, "since", s_since)) {
			try {
				since = boost::lexical_cast<uint64>(s_since);
			}
			catch (boost::bad_lexical_cast const&) {
				response.push_back(json_spirit::Pair("success", false));
				response.push_back(json_spirit::Pair("error_msg", "Invalid 'since' parameter."));
				mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
				mg_printf(conn, write(response).c_str());
				return true;
			}
		}
		if (CivetServer::getParam(conn, "node_id", s_node_id)) {
			try {
				node_id = boost::lexical_cast<uint64>(s_node_id);
			}
			catch (boost::bad_lexical_cast const&) {
				response.push_back(json_spirit::Pair("success", false));
				response.push_back(json_spirit::Pair("error_msg", "Invalid 'node_id' parameter."));
				mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
				mg_printf(conn, write(response).c_str());
				return true;
			}
		}

		response.push_back(json_spirit::Pair("success", true));
		response.push_back(json_spirit::Pair("data", ZWaveController::GetValueChangesSince(since, node_id)));
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
		mg_printf(conn, write(response).c_str());
		return true;
	}

	if (url == "/api/set_value"){

		Logger::LogNotice("API request from " + remote_addr + " for url " + url);

		string s_value_id;
		uint64 value_id = 0;
		if (CivetServer::getParam(conn, "value_id", s_value_id)) {
			try {
				value_id = boost::lexical_cast<uint64>(s_value_id);
			}
			catch (boost::bad_lexical_cast const&) {
				response.push_back(json_spirit::Pair("success", false));
				response.push_back(json_spirit::Pair("error_msg", "Invalid 'value_id' parameter."));
				mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
				mg_printf(conn, write(response).c_str());
				return true;
			}
		}
		else {
			response.push_back(json_spirit::Pair("success", false));
			response.push_back(json_spirit::Pair("error_msg", "Missing 'value_id' parameter."));
			mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
			mg_printf(conn, write(response).c_str());
			return true;
		}

		string new_value;
		if (!CivetServer::getParam(conn, "new_value", new_value)) {
			response.push_back(json_spirit::Pair("success", false));
			response.push_back(json_spirit::Pair("error_msg", "Missing 'new_value' parameter."));
			mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
			mg_printf(conn, write(response).c_str());
			return true;
		}

		string err_message;
		bool success = ZWaveController::SetValue(value_id, new_value, err_message);
		if (!success){
			response.push_back(json_spirit::Pair("success", false));
			response.push_back(json_spirit::Pair("error_msg", err_message));
		}
		else {
			response.push_back(json_spirit::Pair("success", true));
		}
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
		mg_printf(conn, write(response).c_str());
		return true;
	}

	if (url == "/api/refresh_value"){

		Logger::LogNotice("API request from " + remote_addr + " for url " + url);

		string s_value_id;
		uint64 value_id = 0;
		if (CivetServer::getParam(conn, "value_id", s_value_id)) {
			try {
				value_id = boost::lexical_cast<uint64>(s_value_id);
			}
			catch (boost::bad_lexical_cast const&) {
				response.push_back(json_spirit::Pair("success", false));
				response.push_back(json_spirit::Pair("error_msg", "Invalid 'value_id' parameter."));
				mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
				mg_printf(conn, write(response).c_str());
				return true;
			}
		}
		else {
			response.push_back(json_spirit::Pair("success", false));
			response.push_back(json_spirit::Pair("error_msg", "Missing 'value_id' parameter."));
			mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
			mg_printf(conn, write(response).c_str());
			return true;
		}

		string err_message;
		bool success = ZWaveController::RefreshValue(value_id, err_message);
		if (!success){
			response.push_back(json_spirit::Pair("success", false));
			response.push_back(json_spirit::Pair("error_msg", err_message));
		}
		else {
			response.push_back(json_spirit::Pair("success", true));
		}
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
		mg_printf(conn, write(response).c_str());
		return true;
	}

	if (url == "/api/get_value"){

		Logger::LogNotice("API request from " + remote_addr + " for url " + url);

		string s_value_id;
		uint64 value_id = 0;
		if (CivetServer::getParam(conn, "value_id", s_value_id)) {
			try {
				value_id = boost::lexical_cast<uint64>(s_value_id);
			}
			catch (boost::bad_lexical_cast const&) {
				response.push_back(json_spirit::Pair("success", false));
				response.push_back(json_spirit::Pair("error_msg", "Invalid 'value_id' parameter."));
				mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
				mg_printf(conn, write(response).c_str());
				return true;
			}
		}
		else {
			response.push_back(json_spirit::Pair("success", false));
			response.push_back(json_spirit::Pair("error_msg", "Missing 'value_id' parameter."));
			mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
			mg_printf(conn, write(response).c_str());
			return true;
		}

		string err_message;
		response.push_back(json_spirit::Pair("success", true));
		response.push_back(json_spirit::Pair("data", ZWaveController::GetValue(value_id)));
		mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
		mg_printf(conn, write(response).c_str());
		return true;
	}

	response.push_back(json_spirit::Pair("success", false));
	response.push_back(json_spirit::Pair("error_msg", "No content found at that URL"));
	mg_printf(conn, "HTTP/1.1 404 Not Found\r\nContent-Type: application/json\r\n\r\n");
	mg_printf(conn, write(response).c_str());
	return true;
}

//public:

bool API::handleGet(CivetServer *server, struct mg_connection *conn) {
	return API::handleAll("GET", server, conn);
}
bool API::handlePost(CivetServer *server, struct mg_connection *conn) {
	return API::handleAll("POST", server, conn);
}


void API::Initialize(Config &config){

	Logger::LogNotice("Starting http api");

	API::config = config;

	string port = config.GetConfig("api_port");
	const char * options[] = { "document_root", ".",
		"listening_ports", port.c_str(), 0
	};


	API::civit_server = new CivetServer(options);

	API::civit_server->addHandler("**", new API());

	Logger::LogNotice("http api successfully started");
}


