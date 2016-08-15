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

// PARALLEL_MPI
// This file describes definitions used for distributed parallel programming 
// using MPI.

#ifndef PARALLEL_MPI_H
#define PARALLEL_MPI_H

// MPI tags for MPI_Send, MPI_Recv, etc.
#define MPI_QUALNET_EVENT 1
#define MPI_QUALNET_BARRIER 2
#define MPI_QUALNET_SOPS_RPC 3
#define MPI_QUALNET_PIPELINE 4 

#endif /* PARALLEL_MPI_H */
