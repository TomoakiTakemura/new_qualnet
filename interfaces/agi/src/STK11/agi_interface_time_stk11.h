#pragma once

#include "AgStkCommUtil_v11.tlh"
#include "..\agi_interface_time_impl.h"

class CAgiInterfaceTimeStk11 : public CAgiInterfaceTimeImpl<AgStkCommUtilLib::IAgStkCommUtilTime>
{
public:
    CAgiInterfaceTimeStk11();
    CAgiInterfaceTimeStk11(clocktype time);
    CAgiInterfaceTimeStk11(const CAgiInterfaceTimeStk11& rhs);
    ~CAgiInterfaceTimeStk11();

    STDMETHOD(Clone)(AgStkCommUtilLib::IAgStkCommUtilTime** ppClone);

protected:
    virtual AgStkCommUtilLib::IAgStkCommUtilTime* ConstructNew(clocktype time);
};
