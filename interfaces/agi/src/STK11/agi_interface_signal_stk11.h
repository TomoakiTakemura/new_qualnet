#pragma once

#include "AgStkCommUtil_v11.tlh"
#include "..\agi_interface_signal_impl.h"

class CAgiInterfaceSignalStk11 : public CAgiInterfaceSignalImpl<AgStkCommUtilLib::IAgStkCommUtilSignal>
{
public:
    CAgiInterfaceSignalStk11();
    CAgiInterfaceSignalStk11(const CAgiInterfaceSignalStk11& rhs);
    ~CAgiInterfaceSignalStk11();
};
