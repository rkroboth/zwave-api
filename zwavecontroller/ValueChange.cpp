#pragma once
#include "stdafx.h"
#include "ValueChange.h"

#include "Utils.h"
#include "ZWaveController.h"

//private:
std::vector<ValueChange> ValueChange::value_changes;

//public:

void ValueChange::remove_change(uint64 value_id){
	for (std::vector<ValueChange>::iterator i = ValueChange::value_changes.begin(); i != ValueChange::value_changes.end(); ++i){
		ValueChange vc = *i;
		if (vc.value_id == value_id){
			ValueChange::value_changes.erase(i);
			break;
		}
	}
}

void ValueChange::add_change(uint64 value_id){

	ValueChange vc;
	vc.time_added = Utils::GetTimeMilliseconds();
	vc.value_id = value_id;
	ValueChange::value_changes.push_back(vc);

}

void ValueChange::remove_expired_value_changes(){

	uint64 current_milliseconds  = Utils::GetTimeMilliseconds();
	uint64 expire_time = current_milliseconds - ZWaveController::expire_seconds;

	while (1){
		bool found = false;
		for (std::vector<ValueChange>::iterator i = ValueChange::value_changes.begin(); i != ValueChange::value_changes.end(); ++i){
			ValueChange vc = *i;
			if (vc.time_added <= expire_time){
				ValueChange::value_changes.erase(i);
				found = true;
				break;
			}
		}
		if (!found){
			break;
		}
	}

}

