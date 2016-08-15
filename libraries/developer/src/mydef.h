#ifndef MYDEF_H
#define MYDEF_H

#include "types.h"

//#define CHANGE_BI
#define MODE15_4E

typedef
struct struct_myapi_add_beacon_data{
	bool isVariableBeacon;
	UInt8 VariableBO;//if RecnBO not set, set 0
}MyApiBeaconData;

#endif
