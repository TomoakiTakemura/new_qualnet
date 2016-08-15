// Copyright (c) 2001-2015, SCALABLE Network Technologies, Inc.  All Rights Reserved.
//                          600 Corporate Pointe
//                          Suite 1200
//                          Culver City, CA 90230
//                          info@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.


// \defgroup Package_Library_Information Library Information

// \file
// \ingroup Package_Library_Information
// This file describes the APIs used to determined some
// library related information.

#include <stdio.h>
#include <stdlib.h>

#include "library_info.h"

// Whether MultiMedia & Enterprise Library is enabled
// in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_MultimediaEnterpriseLibCompiled()
{
#ifdef ENTERPRISE_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether Wireless Library is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_WirelessLibCompiled()
{
#ifdef WIRELESS_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether Advanced Wireless Library is enabled
// in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_AdvancedWirelessLibCompiled()
{
#ifdef ADVANCED_WIRELESS_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether Ale/Asaps Library is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_AleAsapsLibCompiled()
{
#ifdef ALE_ASAPS_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether Cellular Library is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_CellularLibCompiled()
{
#ifdef CELLULAR_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether Cyber Library is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_CyberLibCompiled()
{
#ifdef CYBER_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether Sensor Networks Library is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_SensorNetworksLibCompiled()
{
#ifdef SENSOR_NETWORKS_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether TIREM Library is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_TiremLibCompiled()
{
#ifdef TIREM_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether UMTS Library is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_UmtsLibCompiled()
{
#ifdef UMTS_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether Urban Propagation Library is enabled
// in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_UrbanPropLibCompiled()
{
#ifdef URBAN_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether Military Radios Library is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_MilitaryRadiosLibCompiled()
{
#ifdef MILITARY_RADIOS_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether AGI interface is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_AgiInterfaceCompiled()
{
#ifdef AGI_INTERFACE
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether LTE is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_LteCompiled()
{
#ifdef LTE_LIB
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether VRLINK is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_VRlinkCompiled()
{
#ifdef VRLINK_INTERFACE
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether SocketInterface is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_SocketInterfaceCompiled()
{
#ifdef SOCKET_INTERFACE
    return TRUE;
#else
    return FALSE;
#endif
}

// Whether Sops/Vops Interface is enabled in compilation
//
//
// \return TRUE if enabled, FALSE otherwise
BOOL INFO_ScenarioPlayerLibCompiled()
{
#ifdef SOPSVOPS_INTERFACE
    return TRUE;
#else
    return FALSE;
#endif
}
