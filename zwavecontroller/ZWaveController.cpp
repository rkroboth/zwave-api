#include "stdafx.h"
#include "ZWaveController.h"

#include "API.h"
#include "Logger.h"

//public:
uint32 ZWaveController::homeId;
bool   ZWaveController::initial_node_queries_complete;
bool   ZWaveController::init_failed;
Config ZWaveController::config;
bool   ZWaveController::running;

uint64 ZWaveController::expire_seconds = 86400000; // 24 hours

void ZWaveController::start(string command_line){

	ZWaveController::running = true;

	InitializeCriticalSection(&g_criticalSection);

	Config config = Config::LoadConfig(command_line);

	Logger::Initialize(config.GetConfig("log_dir"), config.GetConfig("logfile_prefix"), ".log");

	API::Initialize(config);

	ZWaveController::Initialize(config);

}

void ZWaveController::Initialize(Config &config){

	ZWaveController::homeId = 0;
	ZWaveController::initial_node_queries_complete = false;
	ZWaveController::init_failed = false;

	ZWaveController::config = config;

	// Create the OpenZWave Manager.
	// The first argument is the path to the config files (where the manufacturer_specific.xml file is located
	// The second argument is the path for saved Z-Wave network state and the log file.  If you leave it NULL 
	// the log file will appear in the program's working directory.
	string zwave_manufacturer_xml_dir = ZWaveController::config.GetConfig("zwave_manufacturer_xml_dir");
	string data_dir = ZWaveController::config.GetConfig("data_dir");
	Options::Create(zwave_manufacturer_xml_dir, data_dir, "");

	Options::Get()->AddOptionInt("SaveLogLevel", LogLevel_Debug);
	Options::Get()->AddOptionInt("QueueLogLevel", LogLevel_Debug);
	Options::Get()->AddOptionInt("DumpTrigger", LogLevel_Debug);
	Options::Get()->AddOptionInt("PollInterval", 500);
	Options::Get()->AddOptionBool("IntervalBetweenPolls", true);
	Options::Get()->AddOptionBool("ValidateValueChanges", true);
	Options::Get()->Lock();

	Manager::Create();

	// Add a callback handler to the manager.  The second argument is a context that
	// is passed to the OnNotification method.  If the OnNotification is a method of
	// a class, the context would usually be a pointer to that class object, to
	// avoid the need for the notification handler to be a static.
	Manager::Get()->AddWatcher(OnNotification, new ZWaveController());

	// Add a Z-Wave Driver
	// Modify this line to set the correct serial port for your PC interface.

	string port = "\\\\.\\COM";
	string com_port = config.GetConfig("controller_com_port");
	port += com_port;

	Manager::Get()->AddDriver(port);

}


void ZWaveController::stop(){
	Manager::Destroy();
	Options::Destroy();
	DeleteCriticalSection(&g_criticalSection);
}

json_spirit::Object ZWaveController::GetValueInfo(ValueID v){

	int home_id = (int)v.GetHomeId();
	int node_id = (int)v.GetNodeId();

	string product_name = Manager::Get()->GetNodeProductName(home_id, node_id);
	string product_id = Manager::Get()->GetNodeProductId(home_id, node_id);
	string product_type = Manager::Get()->GetNodeProductType(home_id, node_id);
	string node_type = Manager::Get()->GetNodeType(home_id, node_id);

	json_spirit::Object prop_info;

	prop_info.push_back(json_spirit::Pair("node_id", node_id));

	prop_info.push_back(json_spirit::Pair("node_type", node_type));

	prop_info.push_back(json_spirit::Pair("product_name", product_name));

	prop_info.push_back(json_spirit::Pair("product_id", product_id));

	prop_info.push_back(json_spirit::Pair("product_type", product_type));
	
	uint64 vid = v.GetId();
	string s_vid = boost::lexical_cast<string>(vid);
	prop_info.push_back(json_spirit::Pair("value_id", s_vid));

	prop_info.push_back(json_spirit::Pair("label", Manager::Get()->GetValueLabel(v)));

	prop_info.push_back(json_spirit::Pair("help_text", Manager::Get()->GetValueHelp(v)));

	bool bool_prop_value;
	uint8 int8_prop_value;
	int32 int32_prop_value;
	string s_int32_prop_value;
	int16 int16_prop_value;
	string str_prop_value;

	switch (v.GetType()){
	case ValueID::ValueType::ValueType_Bool:
		prop_info.push_back(json_spirit::Pair("type", "bool"));
		prop_info.push_back(json_spirit::Pair("type_help", "0/1, true/false, on/off"));
		Manager::Get()->GetValueAsBool(v, &bool_prop_value);
		prop_info.push_back(json_spirit::Pair("value", bool_prop_value));
		break;
	case ValueID::ValueType::ValueType_Byte:
		prop_info.push_back(json_spirit::Pair("type", "byte"));
		prop_info.push_back(json_spirit::Pair("type_help", "number from 0 to 255"));
		Manager::Get()->GetValueAsByte(v, &int8_prop_value);
		prop_info.push_back(json_spirit::Pair("value", int8_prop_value));
		break;
	case ValueID::ValueType::ValueType_Short:
		prop_info.push_back(json_spirit::Pair("type", "short"));
		prop_info.push_back(json_spirit::Pair("type_help", "number from 0 to 65,535"));
		Manager::Get()->GetValueAsShort(v, &int16_prop_value);
		prop_info.push_back(json_spirit::Pair("value", int16_prop_value));
		break;
	case ValueID::ValueType::ValueType_Decimal:
	case ValueID::ValueType::ValueType_Int:
		prop_info.push_back(json_spirit::Pair("type", "int32"));
		prop_info.push_back(json_spirit::Pair("type_help", "number from 0 to 4,294,967,295"));
		Manager::Get()->GetValueAsInt(v, &int32_prop_value);

		// do all this checking below because of a bug in openzwave: https://code.google.com/p/open-zwave/issues/detail?id=382
		s_int32_prop_value = boost::lexical_cast<string>(int32_prop_value);
		Manager::Get()->GetValueAsString(v, &str_prop_value);
		if (boost::iequals(s_int32_prop_value, str_prop_value)){
			// bug did not happen
			prop_info.push_back(json_spirit::Pair("value", int32_prop_value));
		}
		else {
			// bug DID happen
			prop_info.push_back(json_spirit::Pair("value", str_prop_value));
		}
		break;
	case ValueID::ValueType::ValueType_List:
		prop_info.push_back(json_spirit::Pair("type", "list"));
		prop_info.push_back(json_spirit::Pair("type_help", "list"));
		Manager::Get()->GetValueAsString(v, &str_prop_value);
		prop_info.push_back(json_spirit::Pair("value", str_prop_value));
		break;
	case ValueID::ValueType::ValueType_Schedule:
		prop_info.push_back(json_spirit::Pair("type", "schedule"));
		prop_info.push_back(json_spirit::Pair("type_help", "schedule"));
		Manager::Get()->GetValueAsString(v, &str_prop_value);
		prop_info.push_back(json_spirit::Pair("value", str_prop_value));
		break;
	case ValueID::ValueType::ValueType_String:
		prop_info.push_back(json_spirit::Pair("type", "string"));
		prop_info.push_back(json_spirit::Pair("type_help", "string"));
		Manager::Get()->GetValueAsString(v, &str_prop_value);
		prop_info.push_back(json_spirit::Pair("value", str_prop_value));
		break;
	case ValueID::ValueType::ValueType_Button:
		prop_info.push_back(json_spirit::Pair("type", "button"));
		prop_info.push_back(json_spirit::Pair("type_help", "button (string)"));
		Manager::Get()->GetValueAsString(v, &str_prop_value);
		prop_info.push_back(json_spirit::Pair("value", str_prop_value));
		break;
	case ValueID::ValueType::ValueType_Raw:
		prop_info.push_back(json_spirit::Pair("type", "raw"));
		prop_info.push_back(json_spirit::Pair("type_help", "raw (string)"));
		Manager::Get()->GetValueAsString(v, &str_prop_value);
		prop_info.push_back(json_spirit::Pair("value", str_prop_value));
		break;
	default:
		prop_info.push_back(json_spirit::Pair("type", "unknown"));
		prop_info.push_back(json_spirit::Pair("type_help", "unknown"));
		prop_info.push_back(json_spirit::Pair("value", ""));
		break;
	}

	Manager::Get()->GetValueAsString(v, &str_prop_value);
	prop_info.push_back(json_spirit::Pair("string value", str_prop_value));

	prop_info.push_back(json_spirit::Pair("max", Manager::Get()->GetValueMax(v)));

	prop_info.push_back(json_spirit::Pair("min", Manager::Get()->GetValueMin(v)));

	prop_info.push_back(json_spirit::Pair("units", Manager::Get()->GetValueUnits(v)));

	prop_info.push_back(json_spirit::Pair("is_polled", Manager::Get()->IsValuePolled(v)));

	prop_info.push_back(json_spirit::Pair("read_only", Manager::Get()->IsValueReadOnly(v)));

	prop_info.push_back(json_spirit::Pair("write_only", Manager::Get()->IsValueWriteOnly(v)));

	prop_info.push_back(json_spirit::Pair("has_been_set", Manager::Get()->IsValueSet(v)));

	switch (v.GetGenre()){
	case ValueID::ValueGenre::ValueGenre_Basic:
		prop_info.push_back(json_spirit::Pair("genre", "basic"));
		break;
	case ValueID::ValueGenre::ValueGenre_User:
		prop_info.push_back(json_spirit::Pair("genre", "user"));
		break;
	case ValueID::ValueGenre::ValueGenre_Config:
		prop_info.push_back(json_spirit::Pair("genre", "config"));
		break;
	case ValueID::ValueGenre::ValueGenre_System:
		prop_info.push_back(json_spirit::Pair("genre", "system"));
		break;
	case ValueID::ValueGenre::ValueGenre_Count:
		prop_info.push_back(json_spirit::Pair("genre", "count"));
		break;
	default:
		prop_info.push_back(json_spirit::Pair("genre", "unknown"));
		break;
	}

	return prop_info;

}


json_spirit::Object ZWaveController::GetValueChangesSince(uint64 since_milliseconds, uint64 only_node_id){

	if (since_milliseconds < ZWaveController::expire_seconds){
		return ZWaveController::GetAllValues(only_node_id);
	}

	json_spirit::Object rtn;

	uint64 current_milliseconds = Utils::GetTimeMilliseconds();
	string s_current_milliseconds = boost::lexical_cast<string>(current_milliseconds);
	rtn.push_back(json_spirit::Pair("time", s_current_milliseconds));

	std::vector<json_spirit::Value> values;

	EnterCriticalSection(&g_criticalSection);

	for (std::vector<ValueChange>::iterator i = ValueChange::value_changes.begin(); i != ValueChange::value_changes.end(); ++i){
		ValueChange vc = *i;
		if (i->time_added >= since_milliseconds && i->time_added <= current_milliseconds - 1){
			for (list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it)
			{
				NodeInfo* nodeInfo = *it;

				if (only_node_id != 0 && only_node_id != (uint64)nodeInfo->m_nodeId){
					continue;
				}


				for (list<ValueID>::iterator it2 = nodeInfo->m_values.begin(); it2 != nodeInfo->m_values.end(); ++it2)
				{
					ValueID v = *it2;
					uint64 vid = v.GetId();

					if (vid == vc.value_id){
						if (ZWaveController::ExposeValue(v)){
							values.push_back(ZWaveController::GetValueInfo(v));
						}
					}
				}
			}
		}
	}

	LeaveCriticalSection(&g_criticalSection);

	rtn.push_back(json_spirit::Pair("values", values));

	return rtn;

}




json_spirit::Object ZWaveController::GetAllValues(uint64 only_node_id){
	
	json_spirit::Object rtn;

	uint64 milliseconds = Utils::GetTimeMilliseconds();
	string s_milliseconds = boost::lexical_cast<string>(milliseconds);
	rtn.push_back(json_spirit::Pair("time", s_milliseconds));

	std::vector<json_spirit::Value> values;

	EnterCriticalSection(&g_criticalSection);

	for (list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it)
	{
		NodeInfo* nodeInfo = *it;

		if (only_node_id != 0 && only_node_id != (uint64)nodeInfo->m_nodeId){
			continue;
		}

		for (list<ValueID>::iterator it2 = nodeInfo->m_values.begin(); it2 != nodeInfo->m_values.end(); ++it2)
		{
			ValueID v = *it2;
			if (ZWaveController::ExposeValue(v)){
				values.push_back(ZWaveController::GetValueInfo(v));
			}
		}

	}

	LeaveCriticalSection(&g_criticalSection);

	rtn.push_back(json_spirit::Pair("values", values));

	return rtn;
}


NodeInfo* ZWaveController::GetNodeInfo(Notification const* _notification) {
	uint32 const homeId = _notification->GetHomeId();
	uint8 const nodeId = _notification->GetNodeId();
	for (list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it)
	{
		NodeInfo* nodeInfo = *it;
		if ((nodeInfo->m_homeId == homeId) && (nodeInfo->m_nodeId == nodeId))
		{
			return nodeInfo;
		}
	}
	return NULL;
}

// Callback that is triggered when a value, group or node changes
void ZWaveController::OnNotification(Notification const* _notification, void* _context){

	// Must do this inside a critical section to avoid conflicts with the main thread
	EnterCriticalSection(&g_criticalSection);

	switch (_notification->GetType())
	{
		case Notification::Type_ValueAdded:
		{
			if (NodeInfo* nodeInfo = GetNodeInfo(_notification))
			{

				// Add the new value to our list
				nodeInfo->m_values.push_back(_notification->GetValueID());

				if (!ZWaveController::init_failed && ZWaveController::initial_node_queries_complete){
					ValueID v = _notification->GetValueID();
					if (ZWaveController::ExposeValue(v)){
						uint64 vid = v.GetId();
						ValueChange::add_change(vid);
					}
				}

			}
			break;
		}

		case Notification::Type_ValueRemoved:
		{
			if (NodeInfo* nodeInfo = GetNodeInfo(_notification))
			{
				// Remove the value from out list
				for (list<ValueID>::iterator it = nodeInfo->m_values.begin(); it != nodeInfo->m_values.end(); ++it)
				{

					if (!ZWaveController::init_failed && ZWaveController::initial_node_queries_complete){
						ValueID v = *it;
						uint64 vid = v.GetId();
						ValueChange::remove_change(vid);
					}

					if ((*it) == _notification->GetValueID())
					{
						nodeInfo->m_values.erase(it);
						break;
					}

				}
			}
			break;
		}

		case Notification::Type_ValueChanged:
		{
			// One of the node values has changed
			if (NodeInfo* nodeInfo = GetNodeInfo(_notification))
			{

				if (!ZWaveController::init_failed && ZWaveController::initial_node_queries_complete){

					// the value that changed
					ValueID v = _notification->GetValueID();

					string new_val;
					Manager::Get()->GetValueAsString(v, &new_val);
					Logger::LogNotice("node " + to_string(v.GetNodeId()) + ", value " + to_string(v.GetId()) + " (" + Manager::Get()->GetValueLabel(v) + "), has been changed to " + new_val);

					ValueChange::remove_expired_value_changes();

					// first remove this value from the change queue if it's already in there
					if (ZWaveController::ExposeValue(v)){
						uint64 vid = v.GetId();
						ValueChange::remove_change(vid);
						ValueChange::add_change(vid);
					}
				}

			}
			break;
		}

		case Notification::Type_Group:
		{
		// One of the node's association groups has changed
		if (NodeInfo* nodeInfo = GetNodeInfo(_notification))
		{
			nodeInfo = nodeInfo;		// placeholder for real action
		}
		break;
		}

		case Notification::Type_NodeAdded:
		{
			// Add the new node to our list
			NodeInfo* nodeInfo = new NodeInfo();

			nodeInfo->m_homeId = _notification->GetHomeId();
			nodeInfo->m_nodeId = _notification->GetNodeId();
			nodeInfo->m_polled = false;
			g_nodes.push_back(nodeInfo);

			Logger::LogNotice("node " + to_string(nodeInfo->m_nodeId) + " added");

			break;
		}

		case Notification::Type_NodeRemoved:
		{
			// Remove the node from our list
			uint32 const homeId = _notification->GetHomeId();
			uint8 const nodeId = _notification->GetNodeId();
			for (list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it)
			{
				NodeInfo* nodeInfo = *it;
				if ((nodeInfo->m_homeId == homeId) && (nodeInfo->m_nodeId == nodeId))
				{
					g_nodes.erase(it);
					delete nodeInfo;
					break;
				}
			}
			break;
		}

		case Notification::Type_NodeEvent:
		{
			// We have received an event from the node, caused by a
			// basic_set or hail message.
			if (NodeInfo* nodeInfo = GetNodeInfo(_notification))
			{
				if (nodeInfo->m_nodeId == 5 || nodeInfo->m_nodeId == 6){
					nodeInfo = nodeInfo;		// placeholder for real action
				}
				else {
					nodeInfo = nodeInfo;		// placeholder for real action
				}
			}
			break;
		}

		case Notification::Type_PollingDisabled:
		{
			if (NodeInfo* nodeInfo = GetNodeInfo(_notification))
			{
				nodeInfo->m_polled = false;
			}
			break;
		}

		case Notification::Type_PollingEnabled:
		{
			if (NodeInfo* nodeInfo = GetNodeInfo(_notification))
			{
				nodeInfo->m_polled = true;
			}
			break;
		}

		case Notification::Type_DriverReady:
		{
			ZWaveController::homeId = _notification->GetHomeId();
			Logger::LogNotice("controller driver is ready");
			break;
		}

		case Notification::Type_DriverFailed:
		{
			ZWaveController::init_failed = true;
			Logger::LogNotice("controller initialization failed. make sure another program is not already controlling the controller");
			break;
		}

		case Notification::Type_AwakeNodesQueried:
		case Notification::Type_AllNodesQueried:
		case Notification::Type_AllNodesQueriedSomeDead:
		{
			ZWaveController::initial_node_queries_complete = true;
			Logger::LogNotice("all zwave node queries complete");
			break;
		}

		case Notification::Type_DriverReset:
		case Notification::Type_NodeNaming:
		case Notification::Type_NodeProtocolInfo:
		case Notification::Type_NodeQueriesComplete:
		default:
		{
		}

	} // end of switch

	LeaveCriticalSection(&g_criticalSection);
}

bool ZWaveController::ExposeValue(ValueID v){

	return true;

	switch (v.GetGenre()){
	case ValueID::ValueGenre::ValueGenre_Basic:
	case ValueID::ValueGenre::ValueGenre_User:
	case ValueID::ValueGenre::ValueGenre_Config:
		return true;

	case ValueID::ValueGenre::ValueGenre_System:
	case ValueID::ValueGenre::ValueGenre_Count:
	default:
		return false;
	}

}

bool ZWaveController::SetValue(uint64 value_id, string new_value, string &error_msg){

	EnterCriticalSection(&g_criticalSection);

	bool success = false;
	bool found = false;
	error_msg = "";

	for (list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it)
	{
		NodeInfo* nodeInfo = *it;

		for (list<ValueID>::iterator it2 = nodeInfo->m_values.begin(); it2 != nodeInfo->m_values.end(); ++it2)
		{
			ValueID v = *it2;
			uint64 vid = v.GetId();
			int node_id = v.GetNodeId();
			if (vid == value_id){

				found = true;

				// make sure it's not a read only prop
				if (Manager::Get()->IsValueReadOnly(v)){
					error_msg = "This is a read only property";
				}

				else {

					bool bool_new_value;
					uint8 int8_new_value;
					int16 int16_new_value;
					int32 int32_new_value;

					switch (v.GetType()){

					case ValueID::ValueType::ValueType_Bool:
						if (
							new_value == ""
							|| new_value == "0"
							|| boost::iequals(new_value, "false")
							|| boost::iequals(new_value, "off")
							|| boost::iequals(new_value, "no")
							){
							bool_new_value = false;
						}
						else {
							bool_new_value = true;
						}
						success = Manager::Get()->SetValue(v, bool_new_value);
						if (!success){
							error_msg = "Call to ZWave library SetValue failed";
						}
						break;

					case ValueID::ValueType::ValueType_Byte:
						try {
							int8_new_value = (uint8)(boost::lexical_cast<int>(new_value));
							success = Manager::Get()->SetValue(v, int8_new_value);
							if (!success){
								error_msg = "Call to ZWave library SetValue failed";
							}
						}
						catch (boost::bad_lexical_cast const&) {
							error_msg = "Type required is byte (number from 0 to 255); could not convert new_value to byte";
						}
						break;

					case ValueID::ValueType::ValueType_Short:
						try {
							int16_new_value = boost::lexical_cast<int16>(new_value);
							success = Manager::Get()->SetValue(v, int16_new_value);
							if (!success){
								error_msg = "Call to ZWave library SetValue failed";
							}
						}
						catch (boost::bad_lexical_cast const&) {
							error_msg = "Type required is short (number from 0 to 65,535); could not convert new_value to short";
						}
						break;

					case ValueID::ValueType::ValueType_Decimal:
					case ValueID::ValueType::ValueType_Int:
						try {
							int32_new_value = boost::lexical_cast<int32>(new_value);
							success = Manager::Get()->SetValue(v, int32_new_value);
							if (!success){
								error_msg = "Call to ZWave library SetValue failed";
							}
						}
						catch (boost::bad_lexical_cast const&) {
							error_msg = "Type required is int32 (number from 0 to 4,294,967,295); could not convert new_value to int32";
						}
						break;

					case ValueID::ValueType::ValueType_List:
						error_msg = "No support for setting 'list' value types yet";
						break;

					case ValueID::ValueType::ValueType_Schedule:
						error_msg = "No support for setting 'schedule' value types yet";
						break;

					case ValueID::ValueType::ValueType_String:
					case ValueID::ValueType::ValueType_Button:
					case ValueID::ValueType::ValueType_Raw:
						success = Manager::Get()->SetValue(v, new_value);
						if (!success){
							error_msg = "Call to ZWave library SetValue failed";
						}

						break;

					default:
						error_msg = "Value is an unknown type, could not set";

					}

				}

				break;
			}
		}

	}

	LeaveCriticalSection(&g_criticalSection);

	if (!found){
		error_msg = "Value id " + to_string(value_id) + " could not be found";
	}

	return success;
}






