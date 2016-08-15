#pragma once
#include "..\agi_interface_util_impl2.h"
#include "agi_interface_time_stk11.h"
#include "agi_interface_id_stk11.h"
#include "agi_interface_signal_stk11.h"
#include "agi_interface_mapping_stk11.h"

class CAgiInterfaceUtilStk11 : public CAgiInterfaceUtilImpl2<CAgiInterfaceIDStk11, CAgiInterfaceTimeStk11,
                                                             CAgiInterfaceSignalStk11, CAgiInterfaceMappingStk11>
{
public:
    CAgiInterfaceUtilStk11();
    ~CAgiInterfaceUtilStk11();
};
