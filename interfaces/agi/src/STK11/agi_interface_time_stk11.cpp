#include <sstream>
#include "agi_interface_time_stk11.h"

using namespace AgStkCommUtilLib;

CAgiInterfaceTimeStk11::CAgiInterfaceTimeStk11()
{
}

CAgiInterfaceTimeStk11::CAgiInterfaceTimeStk11(clocktype time) :
CAgiInterfaceTimeImpl<AgStkCommUtilLib::IAgStkCommUtilTime>(time)
{
}

CAgiInterfaceTimeStk11::CAgiInterfaceTimeStk11(const CAgiInterfaceTimeStk11& rhs) :
CAgiInterfaceTimeImpl<AgStkCommUtilLib::IAgStkCommUtilTime>(rhs)
{
}

CAgiInterfaceTimeStk11::~CAgiInterfaceTimeStk11()
{
}

STDMETHODIMP CAgiInterfaceTimeStk11::Clone(IAgStkCommUtilTime** ppClone)
{
    CAgiInterfaceSmartPtr<IAgStkCommUtilTime> newTime = new CAgiInterfaceTimeStk11(*this);
    return newTime.CopyTo(ppClone);
}

AgStkCommUtilLib::IAgStkCommUtilTime* CAgiInterfaceTimeStk11::ConstructNew(clocktype time)
{
    return new CAgiInterfaceTimeStk11(time);
}
