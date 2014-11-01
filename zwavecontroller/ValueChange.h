#ifndef _VALUECHANGE_H
#define _VALUECHANGE_H

#include "headers.h"

class ValueChange {
private:
public:
	static std::vector< ValueChange > value_changes;
	uint64 time_added;
	uint64 value_id;
	static void remove_change(uint64 value_id);
	static void add_change(uint64 value_id);
	static void remove_expired_value_changes();
};

#endif
