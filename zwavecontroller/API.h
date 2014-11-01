#ifndef _API_H
#define _API_H

#include "headers.h"
#include "Config.h"
#include "ZWaveController.h"

class API : public CivetHandler {
private:
	static CivetServer* civit_server;
	static Config config;
	static bool handleAll(const char * method, CivetServer *server, struct mg_connection *conn);
public:
	bool handleGet(CivetServer *server, struct mg_connection *conn);
	bool handlePost(CivetServer *server, struct mg_connection *conn);
	static void Initialize(Config &config);
	static void Doit();
};

#endif

