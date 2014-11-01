#ifndef _HEADERS_H
#define _HEADERS_H

#include "Windows.h"
#include "Resource.h"
#include <thread>
#include <ctime>

// openzwave
#include "Options.h"
#include "Manager.h"
#include "Driver.h"
#include "Node.h"
#include "Group.h"
#include "Notification.h"
#include "value_classes/ValueStore.h"
#include "value_classes/Value.h"
#include "value_classes/ValueBool.h"
#include "platform/Log.h"

// Civetserver
#include "CivetServer.h"

// Boost
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/log/core.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/expressions.hpp"
#include "boost/log/utility/setup/file.hpp"
#include "boost/log/utility/setup/common_attributes.hpp"

// json spirit
#include "json_spirit.h"

using namespace std;
using namespace OpenZWave;

#endif
