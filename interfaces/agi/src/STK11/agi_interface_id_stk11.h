#pragma once

#include "AgStkCommUtil_v11.tlh"
#include "..\agi_interface_id_impl.h"

class CAgiInterfaceIDStk11 : public CAgiInterfaceIDImpl<AgStkCommUtilLib::IAgStkCommUtilId>
{
public:
    CAgiInterfaceIDStk11();
    CAgiInterfaceIDStk11(const CAgiInterfaceIDStk11& rhs);
    ~CAgiInterfaceIDStk11();

    STDMETHOD(Clone)(AgStkCommUtilLib::IAgStkCommUtilId** ppClone);
};
