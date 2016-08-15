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

#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "api.h"
#include "clock.h"
#include "main.h"
#include "node.h"
#include "partition.h"
#include "scheduler.h"
#include "external.h"
#include "timer_manager.h"
#include "user.h"
#include "context.h"
#include "mimo_tgn_models.h"


#ifdef SOCKET_INTERFACE
#include "socket-interface.h"
#endif 

#ifdef ADDON_NGCNMS
#include "phy_abstract.h"
#define DEBUG_NODE_FAULT 0
#endif

#ifdef ADDON_DB
#include "dbapi.h"
#endif

//#define PRINT_EIGEN_VALUES
//#define MIMO_DEBUG_PRINTS

#define I_METRA_LOS
#define MAX_RICEAN_K_FACTOR_IN_DB 3082.304459

#ifdef PRINT_EIGEN_VALUES
#include <fstream>
#endif

void Node::enterInterface(int intf)
{
  //printf("Entering interface %d/%d\n", nodeId, intf);
  //fflush(stdout);

// These assertions are not safe until all entry and exit points are included
  //assert(currentInterface == InterfaceNone);
  currentInterface = intf;
}

void Node::exitInterface()
{
  //printf("Exiting interface %d/%d\n", nodeId, currentInterface);
  //fflush(stdout);

  //assert(currentInterface != InterfaceNone);
  currentInterface = InterfaceNone;
}

int Node::ifidx()
{
    assert(currentInterface != InterfaceNone);
    return currentInterface;
}

// Get current time at the node
// \return The current time
clocktype Node::getNodeTime() const
{
    return partitionData->getGlobalTime();
}

Node::Node() : initMIMO_done(false), m_sl(this), d_jlm(this) { }

Node::~Node() {
  if (radios != 0) {
    for (int i = 0; i < MAX_NUM_PHYS; i++) {
      if (radios[i]) {
      radios[i] = 0;
      }
    }
    delete radios;
    radios = 0;
  }
}

/*
 * FUNCTION     NODE_CreateNode
 * PURPOSE      Function used to allocate and initialize the nodes on a
 *              partition.
 *
 * Parameters
 *     partitionData: a pre-initialized partition data structure
 *     numNodes:      the number of nodes in this partition
 *     seedVal:       the global random seed
 */
Node* NODE_CreateNode(PartitionData* partitionData,
                      NodeAddress    nodeId,
                      int            partitionId,
                      int            index)
{

#ifdef DEBUG
    printf("Creating node %d on partition %d\n", nodeId, partitionData->partitionId);
#endif
    void* mem = MEM_malloc(sizeof(Node));
    assert(mem != 0);
    memset(mem, 0, sizeof(Node));

    Node* newNode = new (mem) Node();
    newNode->partitionData = partitionData;
    newNode->nodeIndex     = index;
    newNode->numNodes      = partitionData->numNodes;
    newNode->nodeId        = nodeId;
    newNode->partitionId   = partitionId;

    newNode->guiOption     = partitionData->guiOption;

    newNode->startTime        = &partitionData->nodeInput->startSimClock;
    newNode->numberInterfaces = 0;
    newNode->numberChannels   = partitionData->numChannels;
    newNode->propChannel      = partitionData->propChannel;

    newNode->packetTrace      = partitionData->traceEnabled;
    newNode->packetTraceSeqno = 0;

    newNode->globalSeed       = partitionData->seedVal;

    newNode->initMIMO();

    newNode->mobilityData     = partitionData->nodePositions[index].mobilityData;

    // allocate definable size structures to let users change constants
    newNode->propData =
        (PropData*)MEM_malloc(sizeof(PropData) * partitionData->numChannels);
    assert(newNode->propData != NULL);
    memset(newNode->propData, 0,
           sizeof(PropData) * partitionData->numChannels);
    for (int i = 0; i < partitionData->numChannels; i++) {
      newNode->propData[i].phyListening = (bool*) MEM_malloc(sizeof(bool) * MAX_NUM_PHYS);
      assert(newNode->propData[i].phyListening != NULL);
      memset(newNode->propData[i].phyListening, 0, sizeof(bool) * MAX_NUM_PHYS);
      newNode->propData[i].phyListenable = (bool*) MEM_malloc(sizeof(bool) * MAX_NUM_PHYS);
      assert(newNode->propData[i].phyListenable != NULL);
      memset(newNode->propData[i].phyListenable, 0, sizeof(bool) * MAX_NUM_PHYS);
    }

    newNode->phyData = NULL;
    newNode->macData = NULL;
    newNode->networkData.networkVar = NULL;

    memset(newNode->globalData,
           0,
           sizeof(void*) * GlobalData_Count);
    newNode->numAtmInterfaces = 0;
    newNode->atmLayer2Data = NULL;

    newNode->userData = (UserData *) MEM_malloc(sizeof(UserData));
    assert(newNode->userData != NULL);
    memset(newNode->userData, 0, sizeof(UserData));

    newNode->currentInterface = Node::InterfaceNone;


#ifdef SOCKET_INTERFACE
    std::string path;
    D_Hierarchy* h = &partitionData->dynamicHierarchy;

    if (h->IsEnabled() && partitionId == partitionData->partitionId)
    {
        if (h->CreateNodePath(
                newNode,
                "multicastGroups",
                path))
        {
            h->AddObject(
                path,
                new D_MulticastGroups(newNode));
        }
    }
#endif

#ifdef GATEWAY_INTERFACE
    newNode->internetGateway = INVALID_ADDRESS;
    newNode->ipv6InternetGateway.networkType = NETWORK_INVALID;
#endif

#ifdef CYBER_LIB
    newNode->resourceManager = NULL;
    newNode->eavesInfo = NULL;
#endif

#ifdef LTE_LIB
    newNode->epcData = NULL;
#endif // LTE_LIB

    return newNode;
    }


/*
 * FUNCTION     NODE_ProcessEvent
 * PURPOSE      Function used to call the appropriate layer to execute
 *              instructions for the message
 *
 * Parameters
 *     node:         node for which message is to be delivered
 *     msg:          message for which instructions are to be executed
 */
void
NODE_ProcessEvent(Node *node, Message *msg)
{
    SimContext::setCurrentNode(node);

    msg->setSent(false);

    // Debug and trace this message
    MESSAGE_DebugProcess(node->partitionData, node, msg);

    if (msg->timerManager)
    {
        msg->timerManager->scheduleNextTimer();
    }

    if (msg->cancelled)
    {
        MESSAGE_Free(node, msg);
        SimContext::unsetCurrentNode();
        return;
    }

#ifdef CYBER_LIB
    if (node->resourceManager)
    {
        node->resourceManager->timerExpired();
    }
#endif

    if (msg->eventType != MSG_PROP_SignalReleased)
    {
        // These events are not counted to maintain consistent event
        // counts during parallel execution.
        node->partitionData->numberOfEvents++;
    }

    /* This statement should be updated as new layers are incorporated. */
    switch (MESSAGE_GetLayer(msg))
    {
        case PROP_LAYER:
        {
            PROP_ProcessEvent(node, msg);
            break;
        }
        case PHY_LAYER:
        {
            PHY_ProcessEvent(node, msg);
            break;
        }
        case MAC_LAYER:
        {
            MAC_ProcessEvent(node, msg);
            break;
        }
        case NETWORK_LAYER:
        {
            NETWORK_ProcessEvent(node, msg);
            break;
        }
        case TRANSPORT_LAYER:
        {
            TRANSPORT_ProcessEvent(node, msg);
            break;
        }
        case QOS_LAYER:
        {
            APP_ProcessEvent(node, msg);
            break;
        }
        case APP_LAYER:
        {
            APP_ProcessEvent(node, msg);
            break;
        }
#ifdef CELLULAR_LIB
        case USER_LAYER:
        {
            USER_ProcessEvent(node, msg);
            break;
        }
#endif // CELLULAR_LIB
        case EXTERNAL_LAYER:
        {
            EXTERNAL_ProcessEvent(node, msg);
            break;
        }
        case WEATHER_LAYER:
        {
            WEATHER_ProcessEvent(node, msg);
            break;
        }
        case ATM_LAYER2:
        {
            ATMLAYER2_ProcessEvent(node, msg);
            break;
        }
        case ADAPTATION_LAYER:
        {
            ADAPTATION_ProcessEvent(node, msg);
            break;
        }
#ifdef WIRELESS_LIB
        case BATTERY_MODEL:
        {
            BatteryProcessEvent(node, msg);
            break;
        }
#endif // WIRELESS_LIB
#if defined(SATELLITE_LIB)
        case UTIL_LAYER:
        {
            extern void UTIL_ProcessEvent(Node*, Message*);
            UTIL_ProcessEvent(node, msg);
            break;
        }
#endif /* SATELLITE_LIB */
#ifdef ADDON_DB
        case STATSDB_LAYER:
        {
            STATSDB_ProcessEvent(node->partitionData, msg);
            break;
        }
#endif
        default:
            printf(
                "Received message for unknown layer %d.\n",
                msg->layerType);
            abort();
    }

    SimContext::unsetCurrentNode();
}


/*
 * FUNCTION     NODE_PrintLocation
 * PURPOSE      Prints the node's three dimensional coordinates.
 *
 * Parameters
 *     node:                   the node
 *     coordinateSystemType:   Cartesian or LatLonAlt
 */
void NODE_PrintLocation(Node* node,
                        int   coordinateSystemType) {
    double x,y,z;

    x = node->mobilityData->current->position.common.c1,
    y = node->mobilityData->current->position.common.c2,
    z = node->mobilityData->current->position.common.c3;

    if (coordinateSystemType == CARTESIAN)
    {
        printf("Partition %d, Node %u (%.2f, %.2f, %.2f).\n",
               node->partitionData->partitionId,
               node->nodeId, x, y, z);
    }
    else
    if (coordinateSystemType == LATLONALT)
    {
        printf("Partition %d, Node %u (%.6f, %.6f, %.6f).\n",
               node->partitionData->partitionId,
               node->nodeId, x, y, z);
    }

    fflush(stdout);
}

// Get terrainData pointer.
//
// \param node  the node
//
// \return TerrainData pointer
TerrainData* NODE_GetTerrainPtr(Node* node)
{
    return PARTITION_GetTerrainPtr(node->partitionData);
}

// Legacy functions -- remove in next release
// Using these will give deprecated warnings
clocktype getSimTime(Node* node)
{
    return node->getNodeTime();
}

clocktype TIME_getSimTime(Node* node)
{
    return node->getNodeTime();
}

const spectralBand* Node::getRadioBand(int phyIndex, int channelIndex) 
{
  if (radios == 0) return 0;
  ERROR_Assert(channelIndex == -1, "need to change getRadioBand(int, int) to getRadioBand(int)");
  return radios[phyIndex];
}


void Node::setRadioBand(int phyIndex, double f, double b) 
{
    PartitionData* partitionData = this->partitionData;
    setRadioBand(phyIndex, spectralBand_Square::makeBand(f, b, "Generic Radio", partitionData));
}

void Node::setRadioBand(int phyIndex, const spectralBand* b) {
  ERROR_AssertArgs(phyIndex >= 0 && phyIndex < MAX_NUM_PHYS, "Node:%d phyIndex %d outof range", nodeId, phyIndex);
  if (radios == 0) {
    radios = new const spectralBand*[MAX_NUM_PHYS];
    memset(radios, 0, sizeof(spectralBand*)*(MAX_NUM_PHYS));
  }
  radios[phyIndex] = b;
}

void Node::setRadioListen(int phyIndex, bool on) {
  ERROR_AssertArgs(phyIndex >= 0 && phyIndex < MAX_NUM_PHYS, "Node:%d phyIndex %d out of range", nodeId, phyIndex);
  ERROR_AssertArgs(radios[phyIndex] != 0, "Node:%d Radio:%d null", nodeId, phyIndex);
  ERROR_AssertArgs(radios[phyIndex]->getQualnetChannel() >= 0, "Node:%d Channel on phy %d absent", nodeId, phyIndex);
  if (on) {
    PHY_StartListeningToChannel(this, phyIndex, radios[phyIndex]->getQualnetChannel());
  } else {
    PHY_StopListeningToChannel(this, phyIndex, radios[phyIndex]->getQualnetChannel());
  }
}

bool Node::getRadioListen(int phyIndex) {
  ERROR_AssertArgs(phyIndex >= 0 && phyIndex < MAX_NUM_PHYS, "Node:%d phyIndex %d out of range", nodeId, phyIndex);
  ERROR_AssertArgs(radios[phyIndex] != 0, "Node:%d Radio:%d null", nodeId, phyIndex);
  ERROR_AssertArgs(radios[phyIndex]->getQualnetChannel() >= 0, "Node:%d Channel on phy %d absent", nodeId, phyIndex);
  return PHY_CanListenToChannel(this, phyIndex, radios[phyIndex]->getQualnetChannel());
}


MIMO_Data& Node::getMIMO_Data(int phyIndex) {
  ERROR_AssertArgs(phyIndex >= 0 && phyIndex < MAX_NUM_PHYS, "Node:%d phyIndex %d out of range", nodeId, phyIndex);
  ERROR_AssertArgs(phyData, "Node:%d phyData null", nodeId);
  ERROR_AssertArgs(phyData[phyIndex]->mimoElementCount <= MAX_MIMO_SIZE, "Node:%d phy:%d element count %d -- too big", nodeId, phyIndex, 
      phyData[phyIndex]->mimoElementCount);
  ERROR_AssertArgs(phyData[phyIndex]->mimoElementCount > 0,"Node:%d phy:%d element count %d -- too small", nodeId, phyIndex, 
      phyData[phyIndex]->mimoElementCount); 
  return phyData[phyIndex]->mimoData[phyData[phyIndex]->mimoElementCount-1];
}


complex<double> Node::correlationFn(double D, double AoA, double AS) {
  complex<double> sum = 0.0;
  double ASprime = AS/sqrt(2.0);
#ifdef MIMO_DEBUG_PRINTS
  printf("\nRoots|Pas|Weights\n");
#endif
  for (int i = 0; i < mimo_GQ_order; i++) {
    double phi = mimo_GQ_roots[i];
    double pas = exp(-phi/ASprime);
    complex<double> ePos = exp(complex<double>(0.0, D*sin(phi-AoA)));
    complex<double> eNeg = exp(complex<double>(0.0, D*sin(-phi-AoA)));
#ifdef MIMO_DEBUG_PRINTS
    printf("%f|%f|%f|%f|%f|%f\t", phi, pas, mimo_GQ_weights[i], ePos, eNeg, D);
#endif
    sum += mimo_GQ_weights[i]*(ePos + eNeg)*pas/ASprime/2.0;
  }
#ifdef MIMO_DEBUG_PRINTS
  printf("\nSum: %f + i%f\t", real(sum), imag(sum));
#endif
  return sum;
}

void  Node::setMIMO_Data(int phyIndex, int elementCount, double elementSpace, const spectralBand* band) {
  ERROR_AssertArgs(elementCount <= MAX_MIMO_SIZE, "Node:%d elementCount %d too big", nodeId, elementCount);
  ERROR_AssertArgs(elementCount > 0, "Node:%d element count %d too small", nodeId, elementCount);
  ERROR_AssertArgs(initMIMO_done, "Node:%d no initMIMO done", nodeId);

  if (elementCount <= 0) return;
  if (phyData[phyIndex]->mimoData.size() < (size_t)elementCount) phyData[phyIndex]->mimoData.resize(elementCount);
  MIMO_Data& data = phyData[phyIndex]->mimoData[elementCount-1];
  phyData[phyIndex]->mimoElementCount = elementCount;

  if (data.m_elementCount == elementCount) return;

  data.m_elementCount = elementCount;
  data.m_elementSpace = elementSpace;

  if (band == 0) return;  // can not do anything if there is no specral band, probably older code.
  if (mimo_Model == 0) return;  // no model specified. Can not construct matrix.
  double wavelength = SPEED_OF_LIGHT/band->getFrequency();

  // here we go about populating the correlation matrix.
  // the matrix is Hermetian, with unit diagonal, so build the top part then
  // construct the rest using this property.
  if (mimoCorrMatType == k_Identity) {
      data.m_correlation = MIMO_Matrix_t::Identity(elementCount, elementCount);
  }
  else if (mimoCorrMatType == k_Gq) {
      MIMO_Matrix_t cor;
      cor = MIMO_Matrix_t::Zero(elementCount, elementCount);
      double delta = 2*PI*elementSpace/wavelength;
      for (int i = 0; mimo_Model[i].weight > 0.0; i++) {
          double AoA = mimo_Model[i].AoA*PI/180;
          double  AS = mimo_Model[i].AS*PI/180;
          double  weight = mimo_Model[i].weight;
          for (int r = 0; r < elementCount; r++) {
              for (int c = r + 1; c < elementCount; c++) {
                  double D = delta*(r-c);
                  cor(r,c) += weight*correlationFn(D, AoA, AS);
              }
          }
      }
      MIMO_Matrix_t cor_star = cor.adjoint();
      data.m_correlation = cor + cor_star +
          MIMO_Matrix_t::Identity(elementCount, elementCount);
  }

#ifdef MIMO_DEBUG_PRINTS
  printf("\n node %d: Printing Correlation matrix D=%f ES=%f\n"
         "                   WaveLength=%f\n", nodeId, delta, elementSpace, wavelength);
  for (int r = 0; r < data.m_correlation.rows(); r++) {
      for (int c = 0; c < data.m_correlation.cols(); c++) {
        printf("%f + i%f\t", real(data.m_correlation(r,c)), imag(data.m_correlation(r,c)));
      }
      printf("\n");
  }
#endif

  data.m_LOS.resize(elementCount, elementCount);
  data.m_LOS = MIMO_Matrix_t::Zero(elementCount, elementCount);
  data.m_LOS_weight = mimoRicean_kFactor;
}

void Node::initMIMO() {
  if (initMIMO_done) return;
  initMIMO_done = true;
  BOOL wasFound;
  char buf[MAX_STRING_LENGTH];
  clocktype interval;

  // The propagation matrix for MIMO calculations only updates
  // occasionally during the simulation.  Zero means the matrix 
  // is constant.
  mimoUpdateInterval = partitionData->maxSimClock;
  IO_ReadTime(
      ANY_NODEID,
      ANY_ADDRESS,
      partitionData->nodeInput,
      "MIMO-UPDATE-INTERVAL",
      &wasFound,
      &interval);
  
  if (wasFound)
  {
      if (interval < 0)
      {
          ERROR_ReportWarning("Incorrect value of MIMO-UPDATE-INTERVAL "
              "is configured. It should be greater than zero. Setting the "
              "value to simulation time configured in the scenario.");
      }
      else if (interval == 0)
      {
          interval = partitionData->maxSimClock;
      }
      mimoUpdateInterval = interval;
  }

  mimo_Model = 0;
  IO_ReadString(
      ANY_NODEID,
      ANY_ADDRESS,
      partitionData->nodeInput,
      "MIMO-MODEL",
      &wasFound,
      buf);

  if (wasFound)
  {
      mimo_Model = MIMO_getTGn_Model(buf);
      if (mimo_Model == 0) 
      {
          ERROR_ReportWarningArgs("MIMO model %s not found", buf);
      }
  }

  mimoCorrMatType = k_Gq;
  IO_ReadString(
      ANY_NODEID,
      ANY_ADDRESS,
      partitionData->nodeInput,
      "MIMO-CORRELATION-MATRIX-TYPE",
      &wasFound,
      buf);

  if (wasFound)
  {
#ifdef WINDOWS_OS
#define strcasecmp strcmpi
#endif
      if (strcasecmp(buf, "GQ") == 0)
      {
          mimoCorrMatType = k_Gq;
      }
      else if (strcasecmp(buf, "IDENTITY") == 0)
      {
          mimoCorrMatType = k_Identity;
      }
      else 
      {
          ERROR_ReportWarning("Incorrect value of "
              "MIMO-CORRELATION-MATRIX-TYPE is configured. It should be "
              "either 'GQ' or 'IDENTITY'. Setting the value to 'GQ'.");
      }
  }

  double kFactor_IN_DB = 0.0;
  mimoRicean_kFactor = 1;
  IO_ReadDouble(
      ANY_NODEID,
      ANY_ADDRESS,
      partitionData->nodeInput,
      "MIMO-RICEAN-K-FACTOR",
      &wasFound,
      &kFactor_IN_DB);

  if (wasFound)
  {
      if (kFactor_IN_DB > MAX_RICEAN_K_FACTOR_IN_DB)
      {
          ERROR_ReportWarningArgs("Incorrect value of MIMO-RICEAN-K-FACTOR "
              "(in dB) is configured. It should be less than "
              "%f. Setting the value to 0.", MAX_RICEAN_K_FACTOR_IN_DB);
      }
      else
      {
          mimoRicean_kFactor = NON_DB(kFactor_IN_DB);
      }
  }

  RandomSeed mimoSeed;
  // the SetSeed chooses a seed from a large array.  It does this by creating
  // an array index from nodeId, appId, and instanceId.  Have to create a unique
  // appId for MIMO to give the algorithm a chance to choose a unque seed.
  // Do not construct the content of any of the matricies now, do not know
  // which ones will be needed.
  RANDOM_SetSeed(mimoSeed, globalSeed, nodeId, APP_MIMO, 0);
  mimo_Rand.setSeed(mimoSeed);
  mimo_Rand.setDistributionGaussian(sqrt(0.5));
  mimoHw_nextRebuild = 0;
  mimoHw_interval = mimoUpdateInterval;

  if (mimoCorrMatType == k_Gq) {
      mimo_GQ_coef_t coef;
      mimo_GQ_get_coefs(coef, mimo_GQ_order);
      mimo_GQ_get_roots(coef, mimo_GQ_order);
  }

  if (mimo_Model == 0) {
    mimo_Model = MIMO_getDefaultTGn_Model();
  }

  mimo_Hw = MIMO_Matrix_t::Zero(MAX_MIMO_SIZE*MAX_MIMO_SIZE, 1);
}

const MIMO_Matrix_t& Node::getMIMO_Hw() {
    if (getNodeTime() == 0 || getNodeTime() > mimoHw_nextRebuild) {
    mimoHw_nextRebuild = getNodeTime() + mimoHw_interval;
    for (int r = 0; r < mimo_Hw.rows(); r++) {
      mimo_Hw(r,0) = complex<double>(mimo_Rand.getRandomNumber(),mimo_Rand.getRandomNumber());
    }
  }
  return mimo_Hw;
}

#ifdef I_METRA_LOS
    void Node::MIMO_getLOSMatrix(double delta, MIMO_Matrix_t& los)
    {
        for (int r = 0; r < los.rows(); r++)
        {
            std::complex<double> theta(0.0, delta*r);
            los(r,0) = exp(theta);
        }
    }
#else
    void Node::MIMO_getLOSMatrix(
         double freq,
         double txES,
         double rxES,
         int txEC,
         int rxEC,
         Orientation AOA,
         double distance,
         MIMO_Matrix_t& los)
    {
        double tx_x = 0;
        double tx_y = 0;
        double rx_x = 0;
        double rx_y = 0;
        double d = 0;
        double theta = 0;
        double lambda = 3.0e8/freq;
        double k = 2*PI/lambda;
        double txSpacing = k*txES;
        double rxSpacing = k*rxES;

        for (int i = 0; i < txEC; i++)
        {
            for (int j = 0; j < rxEC; j++)
            {
                tx_y = txSpacing*i;
                rx_x = distance + (rxSpacing*j)*sin((double)AOA.azimuth*IN_RADIAN);
                rx_y = rxSpacing*j*cos((double)AOA.azimuth*IN_RADIAN);
                d = sqrt(std::pow((tx_x-rx_x),2)+std::pow((tx_y-rx_y),2));
                theta = d/k;
                std::complex<double> cn(0, theta);
                los(i,j) = exp(cn);
            }
        }
    }
#endif

void Node::MIMO_getEigenValues(
     PropRxInfo* propRxInfo, 
     int phyIndex,
     Orientation& txDoA,
     Orientation& rxDoA,
     MIMO_Matrix_t& mimoLambda)
{
    if (mimo_Model == 0)
    {
        return;
    }
    MIMO_Data* TxData = propRxInfo->txMsg->m_mimoData;
    if (TxData->m_elementCount == 0) return;
    MIMO_Data& RxData = getMIMO_Data(phyIndex);

    const MIMO_Matrix_t& Hw = getMIMO_Hw();
    MIMO_Matrix_t Rr = RxData.m_correlation;
    MIMO_Matrix_t Rt = TxData->m_correlation;
    double k_factor = mimoRicean_kFactor;

    // LOS modeling
    MIMO_Matrix_t los = MIMO_Matrix_t::Zero(RxData.m_elementCount,
        TxData->m_elementCount);

#ifdef I_METRA_LOS
    // LOS: Outer product method
    MIMO_Matrix_t Rr_los = MIMO_Matrix_t::Zero(RxData.m_elementCount,1);
    MIMO_Matrix_t Rt_los = MIMO_Matrix_t::Zero(TxData->m_elementCount,1);

    double rxDelta = (2*PI*RxData.m_elementSpace*propRxInfo->frequency
        *sin(propRxInfo->rxDOA.azimuth*IN_RADIAN)/SPEED_OF_LIGHT);
    MIMO_getLOSMatrix(rxDelta, Rr_los);
    double txDelta = (2*PI*TxData->m_elementSpace*propRxInfo->frequency
        *sin(propRxInfo->rxDOA.azimuth*IN_RADIAN))/SPEED_OF_LIGHT;
    MIMO_getLOSMatrix(txDelta, Rt_los);

    MIMO_Matrix_t tm = Rt_los.transpose();
    los = Rr_los*tm;

    // Additional component for los
    //std::complex<double> theta(0, 2*PI*propRxInfo->frequency*cos(PI/4));
    //los = exp(theta)*los;
#else
    Coordinates rxP;
    Coordinates txP;
    MOBILITY_ReturnCoordinates(this, &rxP);
    Node* txNPtr = MAPPING_GetNodePtrFromHash(
        partitionData->nodeIdHash,
        propRxInfo->txNodeId);
    MOBILITY_ReturnCoordinates(txNPtr, &txP);
    PropPathProfile profile;
    memset(&profile, 0, sizeof(PropPathProfile));
    COORD_CalcDistanceAndAngle(
        partitionData->terrainData->getCoordinateSystem(),
        &txP,
        &rxP,
        &(profile.distance),
        &(profile.txDOA),
        &(profile.rxDOA));
    MIMO_getLOSMatrix(propRxInfo->frequency,
        TxData->m_elementSpace,
        RxData.m_elementSpace,
        TxData->m_elementCount,
        RxData.m_elementCount,
        propRxInfo->rxDOA,
        profile.distance,
        los);
#endif

    //Rt = Rt * 0;
    //Rr = Rr * 0;

    //NLOS component modeling
    // this is in an unsupported package we should probably write our own
    MIMO_Matrix_t k = Eigen::kroneckerProduct(Rt, Rr);  
    // lower triangle Cholesky decomposition (L L* = k, L* = L transpose, so LLT)
    MIMO_Matrix_t c = k.llt().matrixU();

    MIMO_Matrix_t nlos = MIMO_Matrix_t::Zero(Rr.rows(), Rt.rows());
    int i = 0;
    for (int r = 0; r < Rr.rows(); r++)
    {
        for (int t = 0; t < Rt.rows(); t++)
        {
            for (int x = 0; x < Rr.rows()*Rt.rows(); x++) {
            ERROR_AssertArgs(i < c.rows(), "Node:%d phy:%d i(%d) too big",
                nodeId, phyIndex, i);
            ERROR_AssertArgs(x < c.cols(), "Node:%d phy:%d x(%d) too big",
                nodeId, phyIndex, x);
            ERROR_AssertArgs(x < Hw.rows(),"Node:%d phy:%d x(%d) too big",
                nodeId, phyIndex, x); 
            nlos(r,t) += c(i,x)*Hw(x,0);
            }
            i++;
        }
    }

    // Combine LOS and NLOS components
    MIMO_Matrix_t hh
        = sqrt(k_factor/(k_factor+1))*los + sqrt(1/(k_factor+1))*nlos;

    MIMO_Matrix_t product = hh * hh.adjoint();
    mimoLambda = product.eigenvalues();

#ifdef I_METRA_LOS
    // Truncate the real part of matrix entries to 8 decimal places
    char bufferR[20];
    for (int r = 0; r < mimoLambda.rows(); r++)
    {
        for (int c = 0; c < mimoLambda.cols(); c++)
        {
            sprintf(bufferR, "%1.8f", real(mimoLambda(r,c)));
            double rp = atof(bufferR);
            std::complex<double> st(rp, imag(mimoLambda(r,c)));
            mimoLambda(r,c) = st;
        }
    }
#endif

#ifdef PRINT_EIGEN_VALUES
    ofstream m("eigValues.mat", ios::out | ios::app);
    if (nodeId == 2 && mimoLambda.rows() > 1)
    {
        for (int r = 0; r < mimoLambda.rows(); r++)
        {
            for (int c = 0; c < mimoLambda.cols(); c++)
            {
                char buf[16];
                sprintf (buf, "%1.8f", real(mimoLambda(r,c)));
                m << buf;
                assert(imag(mimoLambda(r,c)) < 1.0e-10);
            }
            m << ("\t");
            m << ("\t");
        }
        m << ("\n");
    }
    flush(m);
    m.close();
#endif
}

// took this from: http://rosettacode.org/wiki/Numerical_integration/Gauss-Legendre_Quadrature
// their example was C, I changed it to use some C++ idioms.
void Node::mimo_GQ_get_coefs(mimo_GQ_coef_t& lcoef, int order) {
  lcoef.resize(order + 1, order + 1);
  lcoef = mimo_GQ_coef_t::Zero(order+1, order+1);
  lcoef(0,0) = 1.0;
  lcoef(1,1) = 1.0;
  for (int n = 2; n <= order; n++) {
    lcoef(n,0) = -(n-1)*lcoef(n-2,0)/n;
    for (int i = 1; i <= n; i++) {
      lcoef(n,i) = ((2*n-1)*lcoef(n-1,i-1) - (n-1)*lcoef(n-2,i))/n;
    }
  }
}

double Node::mimo_GQ_eval(mimo_GQ_coef_t& coef, int n, double x) {
  double s = coef(n,n);
  for (int i = n; i != 0; i--) {
    s = s*x + coef(n, i-1);
  }
  return s;
}

double Node::mimo_GQ_diff(mimo_GQ_coef_t& coef, int n, double x) {
  return n*(mimo_GQ_eval(coef, n, x) - mimo_GQ_eval(coef, n-1, x))/(x*x - 1);
}

void Node::mimo_GQ_get_roots(mimo_GQ_coef_t& coef, int order) {
  mimo_GQ_roots.resize(order);
  mimo_GQ_weights.resize(order);

  for (int i = 1; i <= order; i++) {
      double x = cos(PI*(i - 0.25)/(order + 0.5));
      double x1 = x;
      do {
        x1 = x;
        x -= mimo_GQ_eval(coef, order, x)/mimo_GQ_diff(coef, order, x);
        double delta = fabs(x1 - x);
        if (delta < 1.0e-6) break;
      } while (true);
      mimo_GQ_roots(i-1) = (x + 1.0)*PI/2;
      x1 = mimo_GQ_diff(coef, order, x);
      mimo_GQ_weights(i-1) = 1.0/((1 - x*x)*x1*x1)*PI;
  }
}
