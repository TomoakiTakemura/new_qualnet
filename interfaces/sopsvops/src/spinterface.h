#ifndef _SPINTERFACE_H_
#define _SPINTERFACE_H_

#include "external.h"

/// The EXTERNAL_Function to handle receive for the
/// EXTERNAL_Interface SPInterface. It calls non-blocking
/// reads from each Scenario Player connection.
///
void SPReceive(EXTERNAL_Interface *iface);

/// The EXTERNAL_Function to advance the simulation horizon
///
void SPSimulationHorizon(EXTERNAL_Interface *iface);

#endif
