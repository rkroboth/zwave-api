#ifndef _ZWAVECONTROLLER_H
#define _ZWAVECONTROLLER_H

#include "headers.h"
#include "Config.h"
#include "ValueChange.h"

typedef struct
{
	uint32			m_homeId;
	uint8			m_nodeId;
	bool			m_polled;
	list<ValueID>	m_values;
} NodeInfo;

static list<NodeInfo*> g_nodes;

static CRITICAL_SECTION g_criticalSection;

class ZWaveController {
public:
	static bool running;
	static uint32 homeId;
	static bool init_failed;
	static bool listening;
	static bool initial_node_queries_complete;
	static uint64 expire_seconds;

	static void start(string command_line);
	static void stop();

	static Config config;
	static void Initialize(Config &config);
	static NodeInfo* GetNodeInfo(Notification const* _notification);
	static void OnNotification(Notification const* _notification, void* _context);
	static json_spirit::Object GetValueChangesSince(uint64 since, uint64 node_id);
	static json_spirit::Object GetAllValues(uint64 node_id);
	static json_spirit::Object GetValueInfo(ValueID v);
	static bool SetValue(uint64 value_id, string new_value, string &error_msg);
	static bool ZWaveController::RefreshValue(uint64 value_id, string &error_msg);
	static bool ExposeValue(ValueID v);
};


#endif
