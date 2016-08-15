#pragma once

#include "AgStkCommUtil_v11.tlh"
#include "..\agi_interface_mapping_impl.h"

class CAgiInterfaceMappingStk11 : public CAgiInterfaceMappingImpl<AgStkCommUtilLib::IAgStkCommUtilEntityMapping>
{
public:
    CAgiInterfaceMappingStk11();
    ~CAgiInterfaceMappingStk11();
};
