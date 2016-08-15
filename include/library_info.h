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


/// \defgroup Package_Library_Information Library Information

/// \file
/// \ingroup Package_Library_Information
/// This file describes the APIs used to determined some
/// library related information.


#ifndef _LIBRARY_INFO_H_
#define _LIBRARY_INFO_H_

#include "types.h"

/// Whether MultiMedia & Enterprise Library is enabled
/// in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_MultimediaEnterpriseLibCompiled();

/// Whether Wireless Library is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_WirelessLibCompiled();

/// Whether Advanced Wireless Library is enabled
/// in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_AdvancedWirelessLibCompiled();

/// Whether Ale/Asaps Library is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_AleAsapsLibCompiled();

/// Whether Cellular Library is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_CellularLibCompiled();

/// Whether Cyber Library is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_CyberLibCompiled();

/// Whether Sensor Networks Library is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_SensorNetworksLibCompiled();

/// Whether TIREM Library is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_TiremLibCompiled();

/// Whether UMTS Library is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_UmtsLibCompiled();

/// Whether Urban Propagation Library is enabled
/// in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_UrbanPropLibCompiled();

/// Whether Military Radios Library is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_MilitaryRadiosLibCompiled();

/// Whether AGI interface is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_AgiInterfaceCompiled();

/// Whether LTE is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_LteCompiled();

/// Whether VRLINK is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_VRlinkCompiled();

/// Whether SocketInterface is enabled in compilation
///
///
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_SocketInterfaceCompiled();

/// Whether Sops/Vops Interface is enabled in compilation
/// \return TRUE if enabled, FALSE otherwise
BOOL INFO_ScenarioPlayerLibCompiled();

#endif // _LIBRARY_INFO_H_
