#include "agi_interface_id_stk11.h"

using namespace AgStkCommUtilLib;

CAgiInterfaceIDStk11::CAgiInterfaceIDStk11()
{
}

CAgiInterfaceIDStk11::CAgiInterfaceIDStk11(const CAgiInterfaceIDStk11& rhs) :
CAgiInterfaceIDImpl<AgStkCommUtilLib::IAgStkCommUtilId>(rhs)
{
}

CAgiInterfaceIDStk11::~CAgiInterfaceIDStk11()
{
}

STDMETHODIMP CAgiInterfaceIDStk11::Clone(IAgStkCommUtilId** ppClone)
{
    CAgiInterfaceSmartPtr<IAgStkCommUtilId> newID = new CAgiInterfaceIDStk11(*this);
    return newID.CopyTo(ppClone);
}
