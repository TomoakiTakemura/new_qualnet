#include "agi_interface_util_stk11_create.h"
#include "agi_interface_util_stk11.h"

IAgiInterfaceUtil* CreateSTK11Util()
{
    return new CAgiInterfaceUtilStk11();
}
