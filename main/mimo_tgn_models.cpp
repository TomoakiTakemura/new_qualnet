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


#include <complex>

#include "mimo_tgn_models.h"

static const MIMO_TGn_cluster TGn_LOS[] =
  {
    {0.0, 0.0, 0.0}
  };

static const MIMO_TGn_cluster TGn_modelA[] =
  {
    {1.0, 45.0, 40.0},
    {0.0, 0.0, 0.0}
  };

static const MIMO_TGn_cluster TGn_modelBr[] =
  { 
    {0.6, 4.3, 14.4},
    {0.4, 118.3, 25.2}
  };

static const MIMO_TGn_cluster TGn_modelBt[] =
  {
    {0.6, 225.1, 14.4},
    {0.4, 106.5, 25.4},
    {0.0, 0.0, 0.0}
  };


static const MIMO_TGn_cluster TGn_modelCr[] =
  {
    {0.763, 290.3, 24.6},
    {0.237, 332.3, 22.4},
    {0.0, 0.0, 0.0}
  };

static const MIMO_TGn_cluster TGn_modelCt[] =
  {
    {0.763, 13.5, 24.7},
    {0.237, 56.3, 22.5},
    {0.0, 0.0, 0.0}
  };

static const MIMO_TGn_cluster TGn_modelDr[] = 
  {
    {0.914, 158.9, 27.7},
    {0.082, 320.2, 31.4},
    {0.004, 276.1, 37.4},
    {0.0, 0.0, 0.0}
  };


static const MIMO_TGn_cluster TGn_modelDt[] = 
  {
    {0.914, 332.1, 27.4},
    {0.082,  49.3, 32.1},
    {0.004, 275.9, 36.8},
    {0.0, 0.0, 0.0}
  };

static const MIMO_TGn_cluster TGn_modelEr[] =
  {
    {0.542, 163.7, 35.8},
    {0.387, 251.8, 41.6},
    {0.066,  80.0, 37.4},
    {0.005, 182.0, 40.3},
    {0.0, 0.0, 0.0}
  };

static const MIMO_TGn_cluster TGn_modelEt[] =
  {
    {0.542, 105.6, 36.1},
    {0.387, 293.1, 42.5},
    {0.066,  61.9, 38.0},
    {0.005, 275.7, 38.7},
    {0.0, 0.0, 0.0}
  };

static const MIMO_TGn_cluster TGn_modelFr[] =
  {
    {0.445, 315.1, 48.0},
    {0.403, 180.4, 55.0},
    {0.109,  74.7, 42.0},
    {0.026, 251.5, 28.6},
    {0.012,  68.5, 30.7},
    {0.005, 246.2, 38.2},
    {0.0, 0.0, 0.0}
  };

static const MIMO_TGn_cluster TGn_modelFt[] =
  {
    {0.445,  56.2, 41.6},
    {0.403, 183.7, 55.2},
    {0.109, 153.0, 47.4},
    {0.026, 112.5, 27.2},
    {0.012, 291.0, 33.0},
    {0.005,  62.3, 38.0},
    {0.0, 0.0, 0.0}
  };

const MIMO_TGn_cluster* MIMO_getTGn_Model(const char* name) {
  if (strcmp(name, "LOS") == 0)  return TGn_LOS;
  if (strcmp(name, "TGnA") == 0) return TGn_modelA;
  if (strcmp(name, "TGnBr") == 0) return TGn_modelBr;
  if (strcmp(name, "TGnBt") == 0) return TGn_modelBt;
  if (strcmp(name, "TGnCr") == 0) return TGn_modelCr;
  if (strcmp(name, "TGnCt") == 0) return TGn_modelCt;
  if (strcmp(name, "TGnDr") == 0) return TGn_modelDr;
  if (strcmp(name, "TGnDt") == 0) return TGn_modelDt;
  if (strcmp(name, "TGnEr") == 0) return TGn_modelEr;
  if (strcmp(name, "TGnEt") == 0) return TGn_modelEt;
  if (strcmp(name, "TGnFr") == 0) return TGn_modelFr;
  if (strcmp(name, "TGnFt") == 0) return TGn_modelFt;

  return 0;
}

const MIMO_TGn_cluster* MIMO_getDefaultTGn_Model() {
  return TGn_modelA;
}
