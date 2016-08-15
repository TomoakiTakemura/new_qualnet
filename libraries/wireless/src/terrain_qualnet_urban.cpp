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

#include <vector>
#include <iostream>

#include "api.h"
#include "terrain_qualnet_urban.h"
#include "terrain_qualnet_urban_parser.h"
#include "partition.h"

#define DEBUG   1
#define NODEBUG 0

// The Fresnel zone is considered obstructed by any terrain feature that is
// closer than fresnelWidthFactor times the Fresnel zone 1 radius to the 
// Line of Sight. The default value for this factor is 0.6. The config file 
// parameter URBAN-TERRAIN-FRESNEL-WIDTH-FACTOR can be used to set this from 
// 0.0 to 1.0. When set to 0.0, the simple Line of Sight is used.
static double fresnelWidthFactor = 0.6;

/*
static void ConvertToGCC(int coordinateSystemType,
                         const Coordinates* c,
                         Coordinates* gccCoord) {
    if (coordinateSystemType == LATLONALT)
    {
        if (c->type != GEOCENTRIC_CARTESIAN)
        {
            COORD_ChangeCoordinateSystem(
                GEODETIC,
                c,
                GEOCENTRIC_CARTESIAN,
                gccCoord);
        }
    }
    else {
        *gccCoord = *c;
    }
}
*/
static void ConvertToGeodetic(int coordinateSystemType,
                              const Coordinates* c,
                              Coordinates* geodeticCoord) {
    if (coordinateSystemType == LATLONALT)
    {
        if (c->type == GEOCENTRIC_CARTESIAN)
        {
            COORD_ChangeCoordinateSystem(
                c,
                GEODETIC,
                geodeticCoord);
        }
    }
    else {
        *geodeticCoord = *c;
    }
}

static void ProjectTo2D(int coordinateSystemType,
                        const Coordinates* c,
                        Coordinates* projectedCoord) {
    // assume c is geodetic
    Coordinates temp;
    if (coordinateSystemType == LATLONALT)
    {
        temp = *c;
        temp.common.c3 = 0;
        COORD_ChangeCoordinateSystem(
            GEODETIC,
            &temp,
            GEOCENTRIC_CARTESIAN,
            projectedCoord);
    }
    else {
        *projectedCoord = *c;
        projectedCoord->common.c3 = 0;
    }
}

void QualNetUrbanPathProperties::sortBuildings() {
    // iterate through buildings.  calculate distance, etc. and put in sorted
    // order by distance from source.
    std::set<BuildingID>::const_iterator thisBuilding;
    std::vector<BuildingData>::iterator  thisBuildingData;
    BuildingData buildingData;

    if (m_alreadySorted) return;
    m_alreadySorted = true;

    if (m_numBuildings == 0) return;

    for (thisBuilding = m_pathData->buildingIDs.begin();
            thisBuilding != m_pathData->buildingIDs.end();
            thisBuilding++)
    {
        double distance1, distance2;

        buildingData.building = *thisBuilding;

        COORD_CalcDistance(CARTESIAN, &m_sourceGCC,
                           &(m_pathData->buildingIntersections[*thisBuilding].point1),
                           &distance1);

        COORD_CalcDistance(CARTESIAN, &m_sourceGCC,
                           &(m_pathData->buildingIntersections[*thisBuilding].point2),
                           &distance2);

        if (distance1 <= distance2) {
            buildingData.distance1 = distance1;
            buildingData.distance2 = distance2;
            buildingData.c1 = m_pathData->buildingIntersections[*thisBuilding].point1;
            buildingData.c2 = m_pathData->buildingIntersections[*thisBuilding].point2;
            buildingData.f1 = m_pathData->buildingFaces[*thisBuilding].f1;
            buildingData.f2 = m_pathData->buildingFaces[*thisBuilding].f2;
        }
        else {
            buildingData.distance1 = distance2;
            buildingData.distance2 = distance1;
            buildingData.c1 = m_pathData->buildingIntersections[*thisBuilding].point2;
            buildingData.c2 = m_pathData->buildingIntersections[*thisBuilding].point1;
            buildingData.f1 = m_pathData->buildingFaces[*thisBuilding].f2;
            buildingData.f2 = m_pathData->buildingFaces[*thisBuilding].f1;
        }

        // insert into building list
        for (thisBuildingData = m_buildingList.begin();
                thisBuildingData != m_buildingList.end();
                thisBuildingData++) {
            if (buildingData < *thisBuildingData) {
                break;
            }
        }
        m_buildingList.insert(thisBuildingData, buildingData);
    }
}

void QualNetUrbanPathProperties::sortFoliage() {
    // iterate through buildings and put into vector.

    std::set<BuildingID>::const_iterator thisFoliage;
    FoliageData foliageData;

    if (m_alreadySortedFoliage)
        return;

    m_alreadySortedFoliage = true;

    if (m_numFoliage == 0) return;

    for (thisFoliage = m_pathData->foliageIDs.begin();
            thisFoliage != m_pathData->foliageIDs.end();
            thisFoliage++)
    {

        foliageData.foliage = *thisFoliage;

        COORD_CalcDistance(CARTESIAN,
                           &(m_pathData->foliageIntersections[*thisFoliage].point1),
                           &(m_pathData->foliageIntersections[*thisFoliage].point2),
                           &foliageData.distanceThrough);

        foliageData.foliatedState = m_urbanData->m_foliatedState;
        foliageData.density = m_urbanData->m_foliage[*thisFoliage].density;
        // insert into foliage list
        m_foliageList.push_back(foliageData);
    }
}

//! Calculates the max, min, and avg building heights.
void QualNetUrbanPathProperties::calculateBuildingHeights() {
    if (m_alreadyCalculatedBuildingHeights) {
        return;
    }
    m_alreadyCalculatedBuildingHeights = true;

    if (m_numBuildings == 0) return;

    double sumHeight = 0.0;
    double maxHeight = 0.0;
    double minHeight = 1000.0; // very tall building
    std::set<BuildingID>::const_iterator thisBuilding;

    for (thisBuilding = m_pathData->buildingIDs.begin();
            thisBuilding != m_pathData->buildingIDs.end();
            thisBuilding++)
    {
        double thisHeight = m_urbanData->m_buildings[*thisBuilding].height;
        sumHeight += thisHeight;
        maxHeight = MAX(maxHeight, thisHeight);
        minHeight = MIN(minHeight, thisHeight);
    }

    m_avgBuildingHeight = (sumHeight / m_numBuildings);
    m_maxRoofHeight     = maxHeight;
    m_minRoofHeight     = minHeight;
}

//! Calculates distance to building, separation, street width
void QualNetUrbanPathProperties::calculateBuildingDistances() {
    if (m_alreadyCalculatedBuildingDistances) {
        return;
    }

    m_alreadyCalculatedBuildingDistances = true;

    // sanity check
    if (m_numBuildings == 0) {
        return;
    }
    if (!m_alreadySorted) {
        sortBuildings();
    }

    unsigned int thisBuilding;
    BuildingData data1;
    BuildingData data2;
    double totalSeparation = 0.0;

    data1 = m_buildingList.front();
    data2 = m_buildingList.back();

    m_srcDistanceToBuilding = data1.distance1;
    m_destDistanceToBuilding = (m_distance - data2.distance2);

    if (m_pathData->numBuildings == 1)
    {
        // Because there's only one building, there's no way to measure
        // distance between buildings, so we estimate by taking average
        // of distance from source to building and building to dest.

        m_avgBuildingSeparation = fabs((m_srcDistanceToBuilding +
                                        m_destDistanceToBuilding) / 2.0);
    }
    else
    {
        for (thisBuilding = 0;
                thisBuilding < (m_buildingList.size() - 1);
                thisBuilding++)
        {
            double tempDistance;
            data1 = m_buildingList[thisBuilding];
            data2 = m_buildingList[thisBuilding+1];

            // separation is the 2nd building's 1st face - the first
            // building's 2nd face.

            tempDistance = fabs(data2.distance1 - data1.distance2);

            if (tempDistance > 0) {
                // the buildings are in a line on the path.
                totalSeparation += tempDistance;
            }
            else {
                // if this is negative, it means the buildings are next to each
                // other rather than in a line along the path.  although the
                // fresnel zone is rarely more than 20m wide, we could still
                // have a situation where two rows of buildings on opposite sides
                // of the road are both in the path.
                // Realistically, the current calculation only works when the path
                // hits buildings broadside. If the path intersects corners of
                // buildings at an angle, the avg separation will be recorded as
                // abnormally large.  Probably should use the region avg instead.
                // TBD, for now, ignoring this case.
            }
        }
        m_avgBuildingSeparation =
            MAX((totalSeparation / (m_buildingList.size() - 1)),
                MIN_BUILDING_SEPARATION);
    }

    m_avgStreetWidth = m_avgBuildingSeparation / 2.0;
}

double QualNetUrbanPathProperties::getSrcDistanceToIntersection() {
    // TBD
    if (!m_alreadyCalculatedSrcDistanceToIntersection) {
        m_alreadyCalculatedSrcDistanceToIntersection = true;
    }
    return m_srcDistanceToIntersection;
}


double QualNetUrbanPathProperties::getDestDistanceToIntersection() {
    // TBD
    if (!m_alreadyCalculatedDestDistanceToIntersection) {
        m_alreadyCalculatedDestDistanceToIntersection = true;
    }
    return m_destDistanceToIntersection;
}


double QualNetUrbanPathProperties::getIntersectionAngle() {
    // TBD
    if (!m_alreadyCalculatedIntersectionAngle) {
        m_alreadyCalculatedIntersectionAngle = true;
    }
    return m_intersectionAngle;
}


//! orientation (used by COST-WI and maybe ITU-R)
double QualNetUrbanPathProperties::getRelativeOrientation() {
    if (!m_alreadyCalculatedRelativeOrientation) {
        m_alreadyCalculatedRelativeOrientation = true;

        BuildingData data;
        FeatureFace  face;

        double lineOfSightNormalX;
        double lineOfSightNormalY;
        double lineOfSightNormalZ;
        Coordinates sourceGeodetic;
        Coordinates destGeodetic;
        Coordinates sourceProjected;
        Coordinates destProjected;

        data = m_buildingList.back(); // this should be closest to receiver.

        // the original code looks at the angle facing node2, presumably
        // the receiver.
        // TBD, we have to choose the first building the node isn't inside
        // i.e. not FACE_RECEIVER or FACE_TRANSMITTER
        face = m_urbanData->m_buildings[data.building].faces[data.f2];

        // take angle between face and line of sight in 2D
        // we remove the z component for the line of sight in CARTESIAN
        // we remove the altitude for the line of sight in LATLONALT
        //
        // the cosine of the normal of the face and the line of sight
        // is the sine of the desired angle
        //
        // we assume the normal of the face is perpendicular to z or altitude

        // TBD, we should replace this with a geometry function
        if (m_coordinateSystemType == LATLONALT)
        {
            ConvertToGeodetic(m_coordinateSystemType,
                              &m_sourceGCC,
                              &sourceGeodetic);
            ConvertToGeodetic(m_coordinateSystemType,
                              &m_destGCC,
                              &destGeodetic);
            ProjectTo2D(m_coordinateSystemType,
                        &sourceGeodetic,
                        &sourceProjected);
            ProjectTo2D(m_coordinateSystemType,
                        &destGeodetic,
                        &destProjected);

            lineOfSightNormalX = destProjected.cartesian.x
                                 - sourceProjected.cartesian.x;
            lineOfSightNormalY = destProjected.cartesian.y
                                 - sourceProjected.cartesian.y;
            lineOfSightNormalZ = destProjected.cartesian.z
                                 - sourceProjected.cartesian.z;
        }
        else
        {
            lineOfSightNormalX = m_dest.cartesian.x - m_source.cartesian.x;
            lineOfSightNormalY = m_dest.cartesian.y - m_source.cartesian.y;
            lineOfSightNormalZ = 0.0;
        }

        double magnitude = sqrt(lineOfSightNormalX * lineOfSightNormalX
                                + lineOfSightNormalY * lineOfSightNormalY
                                + lineOfSightNormalZ * lineOfSightNormalZ);

        lineOfSightNormalX /= magnitude;
        lineOfSightNormalY /= magnitude;
        lineOfSightNormalZ /= magnitude;

        // the dot product of normal vectors is the cosine of the angle
        // between them

        double dotProduct = (face.plane.normalX * lineOfSightNormalX)
                            + (face.plane.normalY * lineOfSightNormalY)
                            + (face.plane.normalZ * lineOfSightNormalZ);

        m_relativeOrientation = (fabs(asin(dotProduct)) / IN_RADIAN);
        if (!(m_relativeOrientation >= 0.0 && m_relativeOrientation <= 90.0))
        {
            printf("Orientation angle error: dotProduct=%f, relativeOrientatin=%f\n",
                dotProduct, m_relativeOrientation);
        }
    }
    return m_relativeOrientation;
}


double QualNetUrbanPathProperties::getAvgFoliageHeight() {

    double sumHeight = 0.0;
    std::set<BuildingID>::const_iterator thisFoliage;

    if (m_numFoliage > 0) {
        for (thisFoliage = m_pathData->foliageIDs.begin();
                thisFoliage != m_pathData->foliageIDs.end();
                thisFoliage++)
        {
            double thisHeight = m_urbanData->m_foliage[*thisFoliage].height;
            sumHeight += thisHeight;
        }
        m_avgFoliageHeight = (sumHeight / m_numFoliage);
    }
    else {
        m_avgFoliageHeight = 0.0;
    }

    return m_avgFoliageHeight;
}


void QualNetUrbanPathProperties::print() {
    int thisBuilding;
    int thisFoliage;

    sortBuildings();
    sortFoliage();
    calculateBuildingHeights();
    calculateBuildingDistances();

    std::cout << "Printing path properties" << std::endl;
    std::cout << "  numBuildings = "               << m_numBuildings << std::endl;
    std::cout << "  avgBuildingSeparation = "      << m_avgBuildingSeparation << std::endl;
    std::cout << "  avgBuildingHeight = "          << m_avgBuildingHeight << std::endl;
    std::cout << "  srcDistanceToIntersection = "  << m_srcDistanceToIntersection << std::endl;
    std::cout << "  destDistanceToIntersection = " << m_destDistanceToIntersection << std::endl;
    std::cout << "  intersectionAngle = "          << m_intersectionAngle << std::endl;
    std::cout << "  avgFoliageHeight = "           << m_avgFoliageHeight << std::endl;
    std::cout << "  maxRoofHeight = "              << m_maxRoofHeight << std::endl;
    std::cout << "  minRoofHeight = "              << m_minRoofHeight << std::endl << std::endl;

    std::cout << std::endl << "Printing buildings" << std::endl;
    for (thisBuilding = 0; thisBuilding < m_numBuildings; thisBuilding++)
    {
        BuildingData data = m_buildingList[thisBuilding];

        std::cout << "  building " << thisBuilding << std::endl;
        std::cout << "  id = " << data.building << std::endl;
        std::cout << "  distance1 = " << data.distance1 << std::endl;
        std::cout << "  distance2 = " << data.distance2 << std::endl;
    }

    std::cout << std::endl << "Printing foliage" << std::endl;
    for (thisFoliage = 0; thisFoliage < m_numFoliage; thisFoliage++)
    {
        FoliageData data = m_foliageList[thisFoliage];

        std::cout << "  foliage " << thisFoliage << std::endl;
        std::cout << "  id = " << data.foliage << std::endl;
        std::cout << "  distance = " << data.distanceThrough << std::endl;
        std::cout << "  density = " << data.density << std::endl;
        std::cout << "  state = " << data.foliatedState << std::endl;
    }
}


// LTE-34
// This function initializes the boost::geometries::index rtrees that are used
// for buildings and foliage intersection tests. It is not thread-safe.
// QualNetUrbanTerrainData::initialize() calls it after it parses the
// urban terrain data. That function is called from the main thread
// before partitions are created.
void QualNetUrbanTerrainData::createRtrees()
{
    if (!m_rtreesInitialized)
    {
        for (int i = 0; i < m_numBuildings; i++)
        {
            // Dereference the bounding cube
            Cube* c = &(m_buildings[i].boundingCube);
            // Create a box from the lowerleft and upperright points.
#ifdef BG_USE_3D
            BgBox b(BgPoint(c->x(), c->y(), c->z()), 
                    BgPoint(c->X(), c->Y(), c->Z()));
#else
            BgBox b(BgPoint(c->x(), c->y()), 
                    BgPoint(c->X(), c->Y()));
#endif
            // Put the Value into the rtree.
            m_rtreeBuildings.insert(std::make_pair(b, i));
        }

        for (int i = 0; i < m_numFoliage; i++)
        {
            Cube* c = &(m_foliage[i].boundingCube);
#ifdef BG_USE_3D
            BgBox b(BgPoint(c->x(), c->y(), c->z()), 
                    BgPoint(c->X(), c->Y(), c->Z()));
#else
            BgBox b(BgPoint(c->x(), c->y()), 
                    BgPoint(c->X(), c->Y()));
#endif
            m_rtreeFoliage.insert(std::make_pair(b, i));
        }

        m_rtreesInitialized = true;
    }
}
// LTE-34-end    

QualNetUrbanTerrainData::~QualNetUrbanTerrainData() {
    delete m_buildings;
    delete m_foliage;
    delete m_roadSegments;
    delete m_intersections;
    delete m_parks;
    delete m_stations;
    if (m_roadSegmentVar != NULL) {
        MEM_free(m_roadSegmentVar);
    }
    if (m_parkStationVar != NULL) {
        MEM_free(m_parkStationVar);
    }
}

void QualNetUrbanTerrainData::initialize(
    NodeInput* nodeInput,
    bool       masterProcess)
{
    ParseQualNetUrbanTerrain(m_terrainData,
                             this,
                             nodeInput,
                             masterProcess);

    // Get the Fresnel zone width factor
    BOOL wasFound;
    char buf[MAX_STRING_LENGTH];

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "URBAN-TERRAIN-FRESNEL-WIDTH-FACTOR",
        &wasFound,
        buf);

    if (wasFound) {
        double factor = atof(buf);
        if (factor < 0.0 || factor > 1.0)
        {
            ERROR_ReportWarning(
                "URBAN-TERRAIN-FRESNEL-WIDTH-FACTOR should be a number "
                "between 0.0 and 1.0");
        }
        fresnelWidthFactor = factor;
    }

    if (NODEBUG)
    {
        printf("Using Fresnel Width Factor = %f\n", fresnelWidthFactor);
    }
 
    createRtrees();
}

void QualNetUrbanTerrainData::finalize()
{
    int i, j;

    //Free Road Segments
    for (i = 0; i < m_numRoadSegments; i++) {
        if (m_roadSegments[i].XML_ID != NULL) {
            MEM_free(m_roadSegments[i].XML_ID);
        }
        if (m_roadSegments[i].streetName != NULL) {
            MEM_free(m_roadSegments[i].streetName);
        }
        MEM_free(m_roadSegments[i].vertices);
    }
    if (m_numRoadSegments > 0)
    {
        MEM_free(m_roadSegments);
    }

    //Free Intersections
    for (i = 0; i < m_numIntersections; i++) {
        if (m_intersections[i].XML_ID != NULL) {
            MEM_free(m_intersections[i].XML_ID);
        }
        MEM_free(m_intersections[i].roadSegments);
        if (m_intersections[i].signalGroups != NULL)
        {
            MEM_free(m_intersections[i].signalGroups);
        }
    }
    if (m_numIntersections > 0)
    {
        MEM_free(m_intersections);
    }

    //Free Parks
    for (i = 0; i < m_numParks; i++) {
        if (m_parks[i].XML_ID != NULL) {
            MEM_free(m_parks[i].XML_ID);
        }
        //        if (m_parks[i].parkName != NULL) {
        //            MEM_free(m_parks[i].parkName);
        //        }
        MEM_free(m_parks[i].vertices);
        MEM_free(m_parks[i].exitIntersections);
    }
    if (m_numParks > 0)
    {
        MEM_free(m_parks);
    }

    //Free Stations
    for (i = 0; i < m_numStations; i++) {
        if (m_stations[i].XML_ID != NULL) {
            MEM_free(m_stations[i].XML_ID);
        }
        //        if (m_stations[i].stationName != NULL) {
        //            MEM_free(m_stations[i].stationName);
        //        }
        MEM_free(m_stations[i].vertices);
        MEM_free(m_stations[i].exitIntersections);
    }
    if (m_numStations > 0)
    {
        MEM_free(m_stations);
    }

    //Free Buildings and their Faces
    for (i = 0; i < m_numBuildings; i++) {
        if (m_buildings[i].XML_ID != NULL) {
            MEM_free(m_buildings[i].XML_ID);
        }
        if (m_buildings[i].featureName != NULL) {
            MEM_free(m_buildings[i].featureName);
        }

        for (j = 0; j < m_buildings[i].num_faces; j++) {
            if (m_buildings[i].faces[j].XML_ID != NULL) {
                MEM_free(m_buildings[i].faces[j].XML_ID);
            }
            MEM_free(m_buildings[i].faces[j].vertices);
        }
        MEM_free(m_buildings[i].faces);

    }
    if (m_numBuildings > 0)
    {
        MEM_free(m_buildings);
    }

    //Free Foliage and their Faces
    for (i = 0; i < m_numFoliage; i++) {
        if (m_foliage[i].XML_ID != NULL) {
            MEM_free(m_foliage[i].XML_ID);
        }
        if (m_foliage[i].featureName != NULL) {
            MEM_free(m_foliage[i].featureName);
        }

        for (j = 0; j < m_foliage[i].num_faces; j++) {
            if (m_foliage[i].faces[j].XML_ID != NULL) {
                MEM_free(m_foliage[i].faces[j].XML_ID);
            }
            MEM_free(m_foliage[i].faces[j].vertices);
        }
        MEM_free(m_foliage[i].faces);

    }
    if (m_numFoliage > 0)
    {
        MEM_free(m_foliage);
    }
}

void QualNetUrbanTerrainData::populateRegionData(TerrainRegion* region,
                                                 TerrainRegion::Mode mode) {

    // TBD, haven't calculated street width.
    // The terrain data does not have information on min/maxElevation,
    // any indoor properties, or foliated state.
    int thisOne;

    double maxRoofHeight     = 0.0;
    double minRoofHeight     = 0.0;
    double totalRoofHeight   = 0.0;
    int    numBuildings      = 0;
    double buildingCoverage  = 0.0;
    int    numFoliage      = 0;
    double totalFoliageCoverage   = 0.0;
    double totalFoliageHeight  = 0.0;
    double totalFoliageDensity = 0.0;

    // find all the buildings in the region
    for (thisOne = 0; thisOne < m_numBuildings; thisOne++) {
        if (!region->intersects(&(m_buildings[thisOne].footprint))) {
            if (NODEBUG) {
                printf("building %s is not in the region\n",
                       m_buildings[thisOne].XML_ID);
            }
            continue;
        }
        if (NODEBUG) {
            printf("building %s is in the region\n", m_buildings[thisOne].XML_ID);
        }

        numBuildings++;
        maxRoofHeight = MAX(maxRoofHeight, m_buildings[thisOne].height);
        minRoofHeight = MIN(maxRoofHeight, m_buildings[thisOne].height);
        totalRoofHeight += m_buildings[thisOne].height;
        buildingCoverage += m_buildings[thisOne].getCoverageArea(
            m_terrainData->getCoordinateSystem());
    }
    region->setNumBuildings(numBuildings);
    region->setMaxRoofHeight(maxRoofHeight);
    if (numBuildings > 0) {
        region->setAvgRoofHeight(totalRoofHeight / (double) numBuildings);
        region->setMinRoofHeight(minRoofHeight);
    }
    region->setBuildingCoverage(buildingCoverage / region->getArea());

    if (mode == TerrainRegion::SIMPLE || mode == TerrainRegion::BUILDINGS) {
        return;
    }

    // find all the foliage in the region
    for (thisOne = 0; thisOne < m_numFoliage; thisOne++) {
        if (region->intersects(&(m_foliage[thisOne].footprint))) {
            continue;
        }

        numFoliage++;
        totalFoliageHeight += m_foliage[thisOne].height;
        totalFoliageDensity += m_foliage[thisOne].density;
        totalFoliageCoverage += m_foliage[thisOne].getCoverageArea(
            m_terrainData->getCoordinateSystem());
    }
    region->setFoliageCoverage(totalFoliageCoverage / region->getArea());
    region->setNumFoliage(numFoliage);
    if (numFoliage > 0) {
        region->setAvgFoliageHeight(totalFoliageHeight / (double) numFoliage);
        region->setAvgFoliageDensity(totalFoliageDensity / (double) numFoliage);
    }
}

// The path approximates the Fresnel zone 1. In 3D, the path is represented
// by a convex octahedron using the end points of the Line of Sight and four
// points surrounding the midpoint of the LoS offset from the LoS by the 
// path radius. In order to check for intersections with building bounding 
// cubes and faces, it is not necessary to define lines connecting the 
// midpoints to each other. The shape is defined by eight line segments 
// connecting the endpoints and each of the four midpoints.
//
// For 2D, the path is a rhombus using the LoS endpoints and points 
// perpendicluar to the midpoint offset by the path radius.
//
// When the fresnelWidthFactor is zero, the path becomes a single line
// segment, the LoS itself.
//
void QualNetUrbanTerrainData::constructPath(
    const  Coordinates& source,
    const  Coordinates& dest,
    double pathRadius,
    BgLinestring& path)
{
    // function assumes all coordinates are in Cartesian or GCC

    // Make boost::geometry points
#ifdef BG_USE_3D
    BgPoint startingPoint(source.cartesian.x, source.cartesian.y, source.cartesian.z);
    BgPoint endingPoint(dest.cartesian.x, dest.cartesian.y, dest.cartesian.z);
#else
    BgPoint startingPoint(source.cartesian.x, source.cartesian.y);
    BgPoint endingPoint(dest.cartesian.x, dest.cartesian.y);
#endif

    // Start fresh
    path.clear();
    path.push_back(startingPoint);

    // if the Fresnel Width Factor is zero, the path becomes the
    // simple Line of Sight.
    if (fresnelWidthFactor < SMALL_NUM)
    {
        path.push_back(endingPoint);
        return;
    }

    // The vertices for the octahedron include the end points of the Line
    // of Sight and four points surrounding the midpoint of the LoS. These four
    // surrounding points are a distance of the Fresnel Zone 1 radius times the
    // Fresnel Width Factor parameter from the LoS. Each point lies on a line 
    // that is perpendicular to the LoS and intersecting the midpoint of the
    // LoS. The lines to the middle vertices are perpendicular to each other.
    // The first step to calculating the four middle vertices is to get the
    // unit vector for the LoS.

    // Create a unit vector, U, for the line of sight (LoS).
    double ux = dest.cartesian.x - source.cartesian.x;
    double uy = dest.cartesian.y - source.cartesian.y;
#ifdef BG_USE_3D
    double uz = dest.cartesian.z - source.cartesian.z;
    double magnitude = sqrt(ux * ux + uy * uy + uz * uz);
    uz /= magnitude;
#else
    double magnitude = sqrt(ux * ux + uy * uy);
#endif
    ux /= magnitude;
    uy /= magnitude;

    // The second step is to get the midpoint of the line.
    double midX = source.cartesian.x + ux * magnitude / 2.0;
    double midY = source.cartesian.y + uy * magnitude / 2.0;
#ifdef BG_USE_3D
    double midZ = source.cartesian.z + uz * magnitude / 2.0;
#endif

    // Step three is to get a vector that is perpendicular to the LoS,
    // which means perpendicular to the unit vector, U. The easiest way
    // to get a perpendicular is to rotate the unit vector around
    // the Z axis. At the same time, scale the unit vector by the
    // offset distance from the LoS. 

#ifdef BG_USE_3D
    double tempx = -uy * pathRadius * fresnelWidthFactor;
    double tempy = ux * pathRadius * fresnelWidthFactor;
    double tempz = uz * pathRadius * fresnelWidthFactor;

    // Now vector P can be rotated around U to be parallel with the X/Y plane.
    double uxux = ux * ux;
    double uxuy = ux * uy;
    double uxuz = ux * uz;
    double uyuy = uy * uy;
    double uyuz = uy * uz;
    double sinE = -uz; // Unit vector, hypotenuse == 1.0
    double cosE = sqrt(uxux + uyuy);
    double oneMinusCosE = 1.0 - cosE;
    double px = tempx * (cosE + uxux * oneMinusCosE) 
              + tempy * (uxuy * oneMinusCosE - uz * sinE)
              + tempz * (uxuz * oneMinusCosE + uy * sinE);
    double py = tempx * (uxuy * oneMinusCosE + uz * sinE)
              + tempy * (cosE + uyuy * oneMinusCosE)
              + tempz * (uyuz * oneMinusCosE - ux * sinE);
    // note: pz == 0.0 -- the vector is parallel to the X/Y plane.
#else // 2D
    double px = -uy * pathRadius * fresnelWidthFactor;
    double py = ux * pathRadius * fresnelWidthFactor;
    // 2D doesn't need any more rotation -- everything is always parallel 
    // to the X/Y plane!
#endif

#ifdef BG_USE_3D
    // Step 4 is to rotate vector P 90 degrees around the unit vector
    // to get vector V.
    double vx = px * uxux + py * (uxuy - uz);
    double vy = px * (uxuy + uz) + py * uyuy;
    double vz = px * (uxuz - uy) + py * (uyuz + ux);
#endif

    // Step 5 is to add the remaining vertices to the path. The starting
    // vertex has already been added. The next vertex is offset from the 
    // midpoint of the LoS by vector P. 
#ifdef BG_USE_3D
    path.push_back(BgPoint(midX + px, midY + py, midZ));
#else
    path.push_back(BgPoint(midX + px, midY + py));
#endif
    // The next vertex is the destination point.
    path.push_back(endingPoint);

#ifdef BG_USE_3D
    // The next vertex is the offset from the midpoint of the LoS
    // by vector V.
    path.push_back(BgPoint(midX + vx, midY + vy, midZ + vz));

    // The path must contain continuous points for the lines that
    // trace the geomtry. At this point we have to add the starting point 
    // again to complete the line segment from Mid + V. 
    path.push_back(startingPoint);
    // The next vertex is the midpoint of Los plus the reverse of vector P.
    path.push_back(BgPoint(midX - px, midY - py, midZ));
    // The end point is needed to complete the line from Mid - P.
    path.push_back(endingPoint);
    // The last vertex is Mid - V.
    path.push_back(BgPoint(midX - vx, midY - vy, midZ - vz));
#else
    path.push_back(BgPoint(midX - px, midY - py));
#endif
    // Finally, complete the geometry by adding the starting point to complete
    // last line from Mid - V.
    path.push_back(startingPoint);
}

TerrainPathDataPointer QualNetUrbanTerrainData::getFeaturesOnPath(
    BgLinestring& path,
    bool includeFoliage)
{
    TerrainPathDataPointer pathData(new TerrainPathData());
    int i;

    int numTempFeatures;

    IntersectedPoints ipoints;
    IntersectedFaces  ifaces;

    BuildingID* tempFeatures;
    Coordinates (* tempIntersections)[2];
    FaceIndex (* tempFaces)[2];

    // return the buildings (and associated data) for buildings that
    // intersect this bounding box for the path.

    BgiResult result;
    m_rtreeBuildings.query(
        bgi::intersects(path), std::back_inserter(result));

//#define GEO_DEBUG
#ifdef GEO_DEBUG
    std::cout << result.size() << " buildings on path:" 
              << bg::dsv(path) << std::endl;
#endif

    returnIntersectionBuildings(result,
                                m_buildings,
                                path,
                                &numTempFeatures,
                                &tempFeatures,
                                &tempIntersections,
                                &tempFaces);

    for (i = 0; i < numTempFeatures; i++)
    {
        BuildingID thisBuilding = tempFeatures[i];

        if (NODEBUG) {
            printf("\t%s\n", m_buildings[thisBuilding].XML_ID);
        }

        // This code skips buildings that contain one of the nodes.
        // For buildings, this may be OK, because this segment should be indoor
        // propagation.
        if (tempFaces[i][0] == FACE_RECEIVER
                || tempFaces[i][0] == FACE_TRANSMITTER
                || tempFaces[i][1] == FACE_RECEIVER
                || tempFaces[i][1] == FACE_TRANSMITTER)
        {
            continue;
        }

        // for each of the new buildings, check to see if they're already in the list
        if (pathData->buildingIDs.count(thisBuilding) == 1)
        {
            // it's already in the list, maybe update values
        }
        else { // add new
            pathData->buildingIDs.insert(thisBuilding);
            ipoints.point1 = tempIntersections[i][0];
            ipoints.point2 = tempIntersections[i][1];
            ifaces.f1  = tempFaces[i][0];
            ifaces.f2  = tempFaces[i][1];
            pathData->buildingIntersections[thisBuilding] = ipoints;
            pathData->buildingFaces[thisBuilding]         = ifaces;
            pathData->numBuildings++;
        }
    }

    MEM_free(tempFeatures);
    MEM_free(tempIntersections);
    MEM_free(tempFaces);

    if (includeFoliage) {
        // return the foliage (and associated data) for foliage that
        // intersects this line segment

        // LTE-34 Use a similar query to reduce the number of foliage items.

        BgiResult result;
        m_rtreeFoliage.query(
            bgi::intersects(path), std::back_inserter(result));
#ifdef GEO_DEBUG
        std::cout << result.size() << " foliage on path:" 
                    << bg::dsv(path) << std::endl;
#endif

        returnIntersectionBuildings(result,
                                    m_foliage,
                                    path,
                                    &numTempFeatures,
                                    &tempFeatures,
                                    &tempIntersections,
                                    &tempFaces);

        for (i = 0; i < numTempFeatures; i++)
        {
            // For buildings, we skipped this because the indoor segment
            // has to be handled differently, but for foliage, we want
            // to consider the case where the node is inside the foliage.
            // TBD, this will mean removing this and changing the code
            // where we calculate distances through foliage to consider
            // this case. Possibly we should change this for OPAR too,
            // or add a parameter.
            if (tempFaces[i][0] == FACE_RECEIVER
                || tempFaces[i][0] == FACE_TRANSMITTER
                || tempFaces[i][1] == FACE_RECEIVER
                || tempFaces[i][1] == FACE_TRANSMITTER)
            {
                continue;
            }

            BuildingID thisFoliage = tempFeatures[i];

            // for each of the new foliages, check to see if they're
            // already in the list
            if (pathData->foliageIDs.count(thisFoliage) == 1)
            {
                // it's already in the list, maybe update values
            }
            else { // add new
                pathData->foliageIDs.insert(thisFoliage);
                ipoints.point1 = tempIntersections[i][0];
                ipoints.point2 = tempIntersections[i][1];
                ifaces.f1  = tempFaces[i][0];
                ifaces.f2  = tempFaces[i][1];
                pathData->foliageIntersections[thisFoliage] = ipoints;
                pathData->foliageFaces[thisFoliage]         = ifaces;
                pathData->numFoliage++;
            }
        }

        MEM_free(tempFeatures);
        MEM_free(tempIntersections);
        MEM_free(tempFaces);
    }

    return pathData;
}

UrbanPathPropertiesPointer QualNetUrbanTerrainData::getUrbanPathProperties(
    const  Coordinates* c1,
    const  Coordinates* c2,
    double pathRadius,
    bool   includeFoliage,
    PartitionData* partition)
{
    const int CACHE_SIZE = 1024;
    UrbanCacheLine lookupKey;
    UrbanPathPropertiesPointer pathProps;

    Coordinates sourceGCC;
    Coordinates destGCC;

    convertToGCC(c1, &sourceGCC);
    convertToGCC(c2, &destGCC);

    // If the partition data is not available, skip the cache. If the partition data
    // is available but the cache has not been created, create it.
    if (partition)
    {
        if (partition->urbanCache == NULL)
        {
            partition->urbanCache = new UrbanCache(CACHE_SIZE);
        }

        // Try for a cache hit on the line.
        // TBD - Does the lookup need three coords? Leaving out Z for now.
        // Note: lookupKey is used below to insert the path into the cache
        // if it is not found.
        lookupKey.first.first = toUrbanCacheCoord(sourceGCC.common.c1);
        lookupKey.first.second = toUrbanCacheCoord(sourceGCC.common.c2);
        lookupKey.second.first = toUrbanCacheCoord(destGCC.common.c1);
        lookupKey.second.second = toUrbanCacheCoord(destGCC.common.c2);

        if (partition->urbanCache->find(lookupKey, pathProps))
        {
            return pathProps;
        }
    }

    pathProps.reset(
        new QualNetUrbanPathProperties(
                    this, c1, c2,
                    m_terrainData->getCoordinateSystem(),
                    &sourceGCC,
                    &destGCC)
    );

    if ((m_numBuildings == 0) && (m_numFoliage == 0))
    {
        // everything will be 0.
        partition->urbanCache->insert(lookupKey, pathProps);
        return pathProps;
    }

    BgLinestring path;
    constructPath(sourceGCC, destGCC, pathRadius, path);

    TerrainPathDataPointer pathFeatures(
        getFeaturesOnPath(path, includeFoliage));

    if (NODEBUG)
    {
        printf("numbuildings on path = %d\n", pathFeatures->numBuildings);
    }
    pathProps->setNumBuildings(pathFeatures->numBuildings);
    pathProps->setNumFoliage(pathFeatures->numFoliage);
    // The smart pointer is typed with the base class. In order to call the 
    // setPathData function that is defined in the derived class, use the get() 
    // method to return the pointer, then cast it to the derived class.
    static_cast<QualNetUrbanPathProperties*>(pathProps.get())->setPathData(pathFeatures);

    if (partition != NULL)
    {
        partition->urbanCache->insert(lookupKey, pathProps);
    }

    return pathProps;
}

// This function determines if the line from source to dest passes through
// an obstruction. If so, it outputs the intersection points and the face
// index of two faces.
bool QualNetUrbanTerrainData::calculateOverlap(
    int coordinateSystemType,
    const Coordinates& source,
    const Coordinates& dest,
    Building* obstruction,
    bool checkInsideOut,
    Coordinates& intersection1,
    Coordinates& intersection2,
    FaceIndex& face1,
    FaceIndex& face2)
{
    if (NODEBUG)
    {
        printf("(%.1f, %.1f, %.1f)-(%.1f, %.1f, %.1f) ",
            source.cartesian.x,
            source.cartesian.y,
            source.cartesian.z,
            dest.cartesian.x,
            dest.cartesian.y,
            dest.cartesian.z);
    }

    // source and dest are in GCC by this point

    bool sourceIsInside = false;
    bool destIsInside   = false;
    int i;
    int intersectionCount;

    face1 = -1; // Init outputs to "no intersection"
    face2 = -1;

    Cube* cube = &(obstruction->boundingCube);
    
    if (NODEBUG) {
        printf("checking building %s with ", obstruction->XML_ID);
        cube->print();
    }

    if (checkInsideOut)
    {
        // TBD, this following check will only work for rectangular buildings
        // We need a geometry function for finding whether a point is inside
        // an arbitrary shape building.
        if ((source.cartesian.x >= cube->x()) &&
            (source.cartesian.y >= cube->y()) &&
            (source.cartesian.z >= cube->z()) &&
            (source.cartesian.x <= cube->X()) &&
            (source.cartesian.y <= cube->Y()) &&
            (source.cartesian.z <= cube->Z()))
        {
            if (NODEBUG) {
                printf("source is within bounds\n");
            }
            sourceIsInside = true;
        }
        if ((dest.cartesian.x >= cube->x()) &&
            (dest.cartesian.y >= cube->y()) &&
            (dest.cartesian.z >= cube->z()) &&
            (dest.cartesian.x <= cube->X()) &&
            (dest.cartesian.y <= cube->Y()) &&
            (dest.cartesian.z <= cube->Z()))
        {
            if (NODEBUG) {
                printf("dest is within bounds\n");
            }
            destIsInside = true;
        }

        if (destIsInside && sourceIsInside) {
            // since both nodes are inside, there's no intersections, though the
            // distance through will of course be the distance.  possibly we should
            // return the two node positions, with FACE_RECEIVER and FACE_TRANSMITTER
            if (NODEBUG)
            {
                printf(" both points inside obstruction.\n");
            }
            return false;
        }
    } // checkInsideOut

    // This function is only reached for obstructions that the path intersected
    // in the rtree query. Now it is time to find which face(s) obstruct the 
    // path.
    intersectionCount = 0;

    for (i = 0; i < obstruction->num_faces; i++)
    {
        if (calculateIntersectionSegmentPlane(
                coordinateSystemType,
                source,
                dest,
                &(obstruction->faces[i]),
                &intersection1))
        {
            if (NODEBUG) {
                printf("face %d is found to intersect at (%f,%f,%f)\n", i,
                       intersection1.cartesian.x,
                       intersection1.cartesian.y,
                       intersection1.cartesian.z);
                obstruction->faces[i].print();
            }
                       
            intersectionCount++;
            face1 = i;
            break;
        }
    }

    i++;
    for (; i < obstruction->num_faces; i++)
    {
        if (calculateIntersectionSegmentPlane(
                coordinateSystemType,
                source,
                dest,
                &(obstruction->faces[i]),
                &intersection2))
        {
            if (NODEBUG) {
                printf("face %d is found to intersect at (%f,%f,%f)\n", i,
                       intersection2.cartesian.x,
                       intersection2.cartesian.y,
                       intersection2.cartesian.z);
                obstruction->faces[i].print();
            }
            // TBD, not sure about this
            // if intersection is on a border of multiple faces
            // it may be repeated
            if ((fabs(intersection1.cartesian.x - intersection2.cartesian.x)
                 < SMALL_NUM)
                && (fabs(intersection1.cartesian.y - intersection2.cartesian.y)
                    < SMALL_NUM)
                && (fabs(intersection1.cartesian.z - intersection2.cartesian.z)
                    < SMALL_NUM))
            {
                continue;
            }

            intersectionCount++;
            face2 = i;
            break;
        }
    }

    if (NODEBUG) {
        printf("source/dest inside = %d/%d\n", sourceIsInside, destIsInside);
    }
    if (intersectionCount == 2)
    {
        return true;
    }
    else if (intersectionCount == 1)
    {
        if (sourceIsInside)
        {
            face2 = FACE_TRANSMITTER;
            memcpy(&intersection2, &source, sizeof(Coordinates));
            return true;
        }
        else if (destIsInside)
        {
            face2 = FACE_RECEIVER;
            memcpy(&intersection2, &dest, sizeof(Coordinates));
            return true;
        }
    }

    return false;
}


//! Returns true if the source/dest line intersects the face, and where.
bool QualNetUrbanTerrainData::calculateIntersectionSegmentPlane(
    int coordinateSystemType,
    const Coordinates& source,
    const Coordinates& dest,
    const FeatureFace* face,
    Coordinates* const intersection)
{
    // assume source/dest, all points in face are in UNREFERENCED_CARTESIAN
    // or GEOCENTRIC_CARTESIAN, depending on the coordinate system

    int    i;
    double mu;
    double dot;
    double tempVector1X;
    double tempVector1Y;
    double tempVector1Z;
    double tempVector2X;
    double tempVector2Y;
    double tempVector2Z;

    // calculate intersection of line with plane

    double denom =
        face->plane.normalX * (dest.cartesian.x - source.cartesian.x)
        + face->plane.normalY * (dest.cartesian.y - source.cartesian.y)
        + face->plane.normalZ * (dest.cartesian.z - source.cartesian.z);
    if (denom == 0)
    {
        // Line and plane don't intersect
        return false;
    }
    mu = - (face->plane.d
            + face->plane.normalX * source.cartesian.x
            + face->plane.normalY * source.cartesian.y
            + face->plane.normalZ * source.cartesian.z)
         / denom;
    if (mu <= 0 || mu >= 1)
    {
        // Intersection not along line segment
        // for mu == 0 or 1
        // treat source/dest as inside when on surface, so no intersection
        return false;
    }

    // Find the 
    intersection->cartesian.x = source.cartesian.x
        + mu * (dest.cartesian.x - source.cartesian.x);
    intersection->cartesian.y = source.cartesian.y
        + mu * (dest.cartesian.y - source.cartesian.y);
    intersection->cartesian.z = source.cartesian.z
        + mu * (dest.cartesian.z - source.cartesian.z);
    if (coordinateSystemType == LATLONALT)
    {
        intersection->type = GEOCENTRIC_CARTESIAN;
    }
    else
    {
        intersection->type = UNREFERENCED_CARTESIAN;
    }

    // check within face on plane

    for (i = 0; i < face->num_vertices; i++)
    {
        // vertex i - intersection
        tempVector1X = face->vertices[i].cartesian.x
                       - intersection->cartesian.x;
        tempVector1Y = face->vertices[i].cartesian.y
                       - intersection->cartesian.y;
        tempVector1Z = face->vertices[i].cartesian.z
                       - intersection->cartesian.z;

        // vertex i+1 - intersection
        tempVector2X =
            face->vertices[(i + 1) % face->num_vertices].cartesian.x
            - intersection->cartesian.x;
        tempVector2Y =
            face->vertices[(i + 1) % face->num_vertices].cartesian.y
            - intersection->cartesian.y;
        tempVector2Z =
            face->vertices[(i + 1) % face->num_vertices].cartesian.z
            - intersection->cartesian.z;

        // dot(normal of plane, cross (vi - inter, vi+1 - inter))
        dot = (tempVector1Y * tempVector2Z - tempVector1Z * tempVector2Y)
              * face->plane.normalX
              + (tempVector1Z * tempVector2X - tempVector1X * tempVector2Z)
              * face->plane.normalY
              + (tempVector1X * tempVector2Y - tempVector1Y * tempVector2X)
              * face->plane.normalZ;

        if (dot == 0)
        {
            // on border of face, including possibly on a vertex
            break;
        }

        if (dot < 0)
        {
            // cross product and normal of plane should be in same direction
            // dot product should be positive
            return false;
        }
    }

    return true;
}

//! Function tells us whether the source/dest line passes through the
//  bounding cube defined in properties.
// Note: this function is no longer used since the boost::geometry rtree
// intersection query already determines that the path passes through the 
// building bounding cube.
/*
bool QualNetUrbanTerrainData::passThroughTest(
    const Coordinates* source,
    const Coordinates* dest,
    Cube* cube)
{
    // Heuristic function
    // return: false indicates that overlap is not possible
    // return: true indicates that overlap is possible
    //         line goes through box defined by max and min x,y,z's
    CoordinateType tempX;
    CoordinateType tempY;
    CoordinateType tempZ;

    if (((cube->X() <= source->cartesian.x) &&
         (cube->X() <= dest->cartesian.x)) ||
        ((cube->x() >= source->cartesian.x) &&
         (cube->x() >= dest->cartesian.x)) ||
        ((cube->Y() <= source->cartesian.y) &&
         (cube->Y() <= dest->cartesian.y)) ||
        ((cube->y() >= source->cartesian.y) &&
         (cube->y() >= dest->cartesian.y)) ||
        ((cube->Z() <= source->cartesian.z) &&
         (cube->Z() <= dest->cartesian.z)) ||
        ((cube->z() >= source->cartesian.z) &&
         (cube->z() >= dest->cartesian.z)))
    {
        return false;
    }

    if (((dest->cartesian.x < cube->x()) &&
         (cube->x() < source->cartesian.x)) ||
        ((source->cartesian.x < cube->x()) &&
         (cube->x() < dest->cartesian.x)))
    {
        // new point at x
        tempY = source->cartesian.y
            + (dest->cartesian.y - source->cartesian.y)
            * (cube->x() - source->cartesian.x)
            / (dest->cartesian.x - source->cartesian.x);
        tempZ = source->cartesian.z
            + (dest->cartesian.z - source->cartesian.z)
            * (cube->x() - source->cartesian.x)
            / (dest->cartesian.x - source->cartesian.x);

        if ((cube->y() <= tempY) && (tempY <= cube->Y()) &&
            (cube->z() <= tempZ) && (tempZ <= cube->Z()))
        {
            return true;
        }
    }

    if (((dest->cartesian.x < cube->X()) &&
         (cube->X() < source->cartesian.x)) ||
        ((source->cartesian.x < cube->X()) &&
         (cube->X() < dest->cartesian.x)))
    {
        // new point at X
        tempY = source->cartesian.y
            + (dest->cartesian.y - source->cartesian.y)
            * (cube->X() - source->cartesian.x)
            / (dest->cartesian.x - source->cartesian.x);
        tempZ = source->cartesian.z
            + (dest->cartesian.z - source->cartesian.z)
            * (cube->X() - source->cartesian.x)
            / (dest->cartesian.x - source->cartesian.x);

        if ((cube->y() <= tempY) && (tempY <= cube->Y()) &&
            (cube->z() <= tempZ) && (tempZ <= cube->Z()))
        {
            return true;
        }
    }

    if (((dest->cartesian.y < cube->y())
         && (cube->y() < source->cartesian.y))
        ||
        ((source->cartesian.y < cube->y())
         && (cube->y() < dest->cartesian.y)))
    {
        // new point at y
        tempX = source->cartesian.x
            + (dest->cartesian.x - source->cartesian.x)
            * (cube->y() - source->cartesian.y)
            / (dest->cartesian.y - source->cartesian.y);
        tempZ = source->cartesian.z
            + (dest->cartesian.z - source->cartesian.z)
            * (cube->y() - source->cartesian.y)
            / (dest->cartesian.y - source->cartesian.y);

        if ((cube->x() <= tempX) && (tempX <= cube->X()) &&
            (cube->z() <= tempZ) && (tempZ <= cube->Z()))
        {
            return true;
        }
    }

    if (((dest->cartesian.y < cube->Y()) &&
         (cube->Y() < source->cartesian.y)) ||
        ((source->cartesian.y < cube->Y()) &&
         (cube->Y() < dest->cartesian.y)))
    {
        // new point at Y
        tempX = source->cartesian.x
            + (dest->cartesian.x - source->cartesian.x)
            * (cube->Y() - source->cartesian.y)
            / (dest->cartesian.y - source->cartesian.y);
        tempZ = source->cartesian.z
            + (dest->cartesian.z - source->cartesian.z)
            * (cube->Y() - source->cartesian.y)
            / (dest->cartesian.y - source->cartesian.y);

        if ((cube->x() <= tempX) && (tempX <= cube->X())
            && (cube->z() <= tempZ) && (tempZ <= cube->Z()))
        {
            return true;
        }
    }

    if (((dest->cartesian.z < cube->z()) &&
         (cube->z() < source->cartesian.z)) ||
        ((source->cartesian.z < cube->z()) &&
         (cube->z() < dest->cartesian.z)))
    {
        // new point at z
        tempX = source->cartesian.x
            + (dest->cartesian.x - source->cartesian.x)
            * (cube->z() - source->cartesian.z)
            / (dest->cartesian.z - source->cartesian.z);
        tempY = source->cartesian.y
            + (dest->cartesian.y - source->cartesian.y)
            * (cube->z() - source->cartesian.z)
            / (dest->cartesian.z - source->cartesian.z);

        if ((cube->x() <= tempX) && (tempX <= cube->X()) &&
            (cube->y() <= tempY) && (tempY <= cube->Y()))
        {
            return true;
        }
    }

    if (((dest->cartesian.z < cube->Z()) &&
         (cube->Z() < source->cartesian.z)) ||
        ((source->cartesian.z < cube->Z()) &&
         (cube->Z() < dest->cartesian.z)))
    {
        // new point at Z
        tempX = source->cartesian.x
            + (dest->cartesian.x - source->cartesian.x)
            * (cube->Z() - source->cartesian.z)
            / (dest->cartesian.z - source->cartesian.z);
        tempY = source->cartesian.y
            + (dest->cartesian.y - source->cartesian.y)
            * (cube->Z() - source->cartesian.z)
            / (dest->cartesian.z - source->cartesian.z);

        if ((cube->x() <= tempX) && (tempX <= cube->X()) &&
            (cube->y() <= tempY) && (tempY <= cube->Y()))
        {
            return true;
        }
    }

    return false;
}
*/

void QualNetUrbanTerrainData::createBoundingCube(Building* building)
{
    FeatureFace* face;
    int i;
    int j;

    building->boundingCube.lower = building->faces[0].vertices[0];
    building->boundingCube.upper = building->faces[0].vertices[0];

    for (i = 0; i < building->num_faces; i++)
    {
        face = &building->faces[i];
        face->boundingCube.lower = face->vertices[0];
        face->boundingCube.upper = face->vertices[0];

        for (j = 1; j < face->num_vertices; j++)
        {
            // convert each face to cartesian, or assume cartesian

            if (face->vertices[j].cartesian.x < face->boundingCube.x())
            {
                face->boundingCube.x() = face->vertices[j].cartesian.x;
            }
            else if (face->vertices[j].cartesian.x > face->boundingCube.X())
            {
                face->boundingCube.X() = face->vertices[j].cartesian.x;
            }

            if (face->vertices[j].cartesian.y < face->boundingCube.y())
            {
                face->boundingCube.y() = face->vertices[j].cartesian.y;
            }
            else if (face->vertices[j].cartesian.y > face->boundingCube.Y())
            {
                face->boundingCube.Y() = face->vertices[j].cartesian.y;
            }

            if (face->vertices[j].cartesian.z < face->boundingCube.z())
            {
                face->boundingCube.z() = face->vertices[j].cartesian.z;
            }
            else if (face->vertices[j].cartesian.z > face->boundingCube.Z())
            {
                face->boundingCube.Z() = face->vertices[j].cartesian.z;
            }
        }

        if (face->boundingCube.x() < building->boundingCube.x())
        {
            building->boundingCube.x() = face->boundingCube.x();
        }
        if (face->boundingCube.X() > building->boundingCube.X())
        {
            building->boundingCube.X() = face->boundingCube.X();
        }
        if (face->boundingCube.y() < building->boundingCube.y())
        {
            building->boundingCube.y() = face->boundingCube.y();
        }
        if (face->boundingCube.Y() > building->boundingCube.Y())
        {
            building->boundingCube.Y() = face->boundingCube.Y();
        }
        if (face->boundingCube.z() < building->boundingCube.z())
        {
            building->boundingCube.z() = face->boundingCube.z();
        }
        if (face->boundingCube.Z() > building->boundingCube.Z())
        {
            building->boundingCube.Z() = face->boundingCube.Z();
        }
    }

    // setting footprint
    if (m_terrainData->getCoordinateSystem() == CARTESIAN
        )
    {
        building->footprint.setMaxX(building->boundingCube.X());
        building->footprint.setMinX(building->boundingCube.x());
        building->footprint.setMaxY(building->boundingCube.Y());
        building->footprint.setMinY(building->boundingCube.y());
    }
    else // LATLONALT
    {
        // Ignore case where building straddles the dateline, for now.
        // I mean, it's mostly ocean, right?
        for (i = 0; i < building->num_faces; i++)
        {
            face = &building->faces[i];
            Coordinates lower;
            Coordinates upper;
            COORD_ChangeCoordinateSystem(&(face->boundingCube.lower),
                                         GEODETIC,
                                         &lower);
            COORD_ChangeCoordinateSystem(&(face->boundingCube.upper),
                                         GEODETIC,
                                         &upper);

            if (i == 0) {
                building->footprint.lower = lower;
                building->footprint.upper = upper;
            }
            else {
                if (lower.latlonalt.latitude < building->footprint.lower.latlonalt.latitude)
                {
                    building->footprint.lower.latlonalt.latitude = lower.latlonalt.latitude;
                }
                if (lower.latlonalt.longitude < building->footprint.lower.latlonalt.longitude)
                {
                    building->footprint.lower.latlonalt.longitude = lower.latlonalt.longitude;
                }
                if (upper.latlonalt.latitude > building->footprint.upper.latlonalt.latitude)
                {
                    building->footprint.upper.latlonalt.latitude = upper.latlonalt.latitude;
                }
                if (upper.latlonalt.longitude > building->footprint.upper.latlonalt.longitude)
                {
                    building->footprint.upper.latlonalt.longitude = upper.latlonalt.longitude;
                }
            }
        }
    }
    if (NODEBUG) {
        printf("building %s footprint\n", building->XML_ID);
        building->footprint.print();
    }
}

void QualNetUrbanTerrainData::createPlaneParameters(Building* building)
{
    int i;

    Coordinates* pointA;
    Coordinates* pointB;
    Coordinates* pointC;

    double tempVector1X;
    double tempVector1Y;
    double tempVector1Z;
    double tempVector2X;
    double tempVector2Y;
    double tempVector2Z;


    for (i = 0; i < building->num_faces; i++)
    {
        // calculate plane for face based on first 3 vertices
        pointA = &(building->faces[i].vertices[0]);
        pointB = &(building->faces[i].vertices[1]);
        pointC = &(building->faces[i].vertices[2]);

        // Pb - Pa
        tempVector1X = pointB->cartesian.x - pointA->cartesian.x;
        tempVector1Y = pointB->cartesian.y - pointA->cartesian.y;
        tempVector1Z = pointB->cartesian.z - pointA->cartesian.z;
        // Pc - Pa
        tempVector2X = pointC->cartesian.x - pointA->cartesian.x;
        tempVector2Y = pointC->cartesian.y - pointA->cartesian.y;
        tempVector2Z = pointC->cartesian.z - pointA->cartesian.z;
        // normal of plane = (Pb - Pa) cross (Pc - Pa)
        building->faces[i].plane.normalX = tempVector1Y * tempVector2Z
                                           - tempVector1Z * tempVector2Y;
        building->faces[i].plane.normalY = tempVector1Z * tempVector2X
                                           - tempVector1X * tempVector2Z;
        building->faces[i].plane.normalZ = tempVector1X * tempVector2Y
                                           - tempVector1Y * tempVector2X;

        double magnitude;
        magnitude= sqrt(building->faces[i].plane.normalX
                        * building->faces[i].plane.normalX
                        + building->faces[i].plane.normalY
                        * building->faces[i].plane.normalY
                        + building->faces[i].plane.normalZ
                        * building->faces[i].plane.normalZ);
        building->faces[i].plane.normalX /= magnitude;
        building->faces[i].plane.normalY /= magnitude;
        building->faces[i].plane.normalZ /= magnitude;

        building->faces[i].plane.d =
            - building->faces[i].plane.normalX * pointA->cartesian.x
            - building->faces[i].plane.normalY * pointA->cartesian.y
            - building->faces[i].plane.normalZ * pointA->cartesian.z;

        if (NODEBUG)
        {
            printf("BLDG=%s FACE=%d Xn=%.5f Yn=%.5f Zn=%.5f d=%.5f\n",
                building->XML_ID, i, 
                building->faces[i].plane.normalX,
                building->faces[i].plane.normalY,
                building->faces[i].plane.normalZ,
                building->faces[i].plane.d);
        }
    }
}

void QualNetUrbanTerrainData::calculateIntersection3Planes(
    const FeatureFace* face1,
    const FeatureFace* face2,
    const FeatureFace* face3,
    Coordinates* const intersection)
{

    // point = -d1*cross(n2,n3)-d2*cross(n3,n1)-d3*cross(n1,n2)
    //          /dot(n1,cross(n2,n3))

    double cross23X = face2->plane.normalY * face3->plane.normalZ
                      - face2->plane.normalZ * face3->plane.normalY;
    double cross23Y = face2->plane.normalZ * face3->plane.normalX
                      - face2->plane.normalX * face3->plane.normalZ;
    double cross23Z = face2->plane.normalX * face3->plane.normalY
                      - face2->plane.normalY * face3->plane.normalX;

    double denom = face1->plane.normalX * cross23X
                   + face1->plane.normalY * cross23Y
                   + face1->plane.normalZ * cross23Z;

    if (fabs(denom) < SMALL_NUM)
    {
        ERROR_ReportError("calculateIntersection3Planes: "
                          "Requested intersection of three planes that do "
                          "not have a unique point intersection.\n");
    }

    intersection->cartesian.x =
        (- face1->plane.d * cross23X
         - face2->plane.d * (face3->plane.normalY * face1->plane.normalZ
                             - face3->plane.normalZ * face1->plane.normalY)
         - face3->plane.d * (face1->plane.normalY * face2->plane.normalZ
                             - face1->plane.normalZ * face2->plane.normalY))
        / denom;

    intersection->cartesian.y =
        (- face1->plane.d * cross23Y
         - face2->plane.d * (face3->plane.normalZ * face1->plane.normalX
                             - face3->plane.normalX * face1->plane.normalZ)
         - face3->plane.d * (face1->plane.normalZ * face2->plane.normalX
                             - face1->plane.normalX * face2->plane.normalZ))
        / denom;

    intersection->cartesian.z =
        (- face1->plane.d * cross23Z
         - face2->plane.d * (face3->plane.normalX * face1->plane.normalY
                             - face3->plane.normalY * face1->plane.normalX)
         - face3->plane.d * (face1->plane.normalX * face2->plane.normalY
                             - face1->plane.normalY * face2->plane.normalX))
        / denom;
}


double QualNetUrbanTerrainData::cosineOfVectors(
    int coordinateSystemType,
    const Coordinates* source,
    const Coordinates* dest1,
    const Coordinates* dest2)
{
    Coordinates save1;
    Coordinates save2;
    Coordinates save3;

    Coordinates vector1;
    Coordinates vector2;

    if (coordinateSystemType == LATLONALT)
    {
        if (source->type != GEOCENTRIC_CARTESIAN)
        {
            COORD_ChangeCoordinateSystem(
                GEODETIC,
                source,
                GEOCENTRIC_CARTESIAN,
                &save1);
            source = &save1;
        }
        if (dest1->type != GEOCENTRIC_CARTESIAN)
        {
            COORD_ChangeCoordinateSystem(
                GEODETIC,
                dest1,
                GEOCENTRIC_CARTESIAN,
                &save2);
            dest1 = &save2;
        }
        if (dest2->type != GEOCENTRIC_CARTESIAN)
        {
            COORD_ChangeCoordinateSystem(
                GEODETIC,
                dest2,
                GEOCENTRIC_CARTESIAN,
                &save3);
            dest2 = &save3;
        }
    }

    vector1.cartesian.x = dest1->cartesian.x - source->cartesian.x;
    vector1.cartesian.y = dest1->cartesian.y - source->cartesian.y;
    vector1.cartesian.z = dest1->cartesian.z - source->cartesian.z;

    vector2.cartesian.x = dest2->cartesian.x - source->cartesian.x;
    vector2.cartesian.y = dest2->cartesian.y - source->cartesian.y;
    vector2.cartesian.z = dest2->cartesian.z - source->cartesian.z;

    double magnitude1 = sqrt(vector1.cartesian.x * vector1.cartesian.x
                             + vector1.cartesian.y * vector1.cartesian.y
                             + vector1.cartesian.z * vector1.cartesian.z);
    double magnitude2 = sqrt(vector2.cartesian.x * vector2.cartesian.x
                             + vector2.cartesian.y * vector2.cartesian.y
                             + vector2.cartesian.z * vector2.cartesian.z);

    if ((magnitude1 < SMALL_NUM) && (magnitude2 < SMALL_NUM))
    {
#if 0
        ERROR_ReportError("cosineOfVectors: "
                          "First, second, and third coordinates are at the "
                          "same location.\n");
#endif
        return COSINE_ERROR_ALL;
    }

    if (magnitude1 < SMALL_NUM)
    {
#if 0
        ERROR_ReportError("cosineOfVectors: "
                          "First and second coordinates are at the same "
                          "location.\n");
#endif
        return COSINE_ERROR_FIRST_SECOND;
    }

    if (magnitude2 < SMALL_NUM)
    {
#if 0
        ERROR_ReportError("cosineOfVectors: "
                          "First and third coordinates are at the same "
                          "location.\n");
#endif
        return COSINE_ERROR_FIRST_THIRD;
    }

    return (vector1.cartesian.x * vector2.cartesian.x
            + vector1.cartesian.y * vector2.cartesian.y
            + vector1.cartesian.z * vector2.cartesian.z)
           / (magnitude1 * magnitude2);
}

double QualNetUrbanTerrainData::calcDistanceOverTerrain(
    const Coordinates* position1,
    const Coordinates* position2,
    double elevationSamplingDistance)
{
    double distance;

    COORD_CalcDistance(
        LATLONALT,
        position1,
        position2,
        &distance);

    if ((m_terrainData->getCoordinateSystem() == LATLONALT)
            && (m_terrainData->hasElevationData()))
    {
        int numSamples = (int) ceil(distance / elevationSamplingDistance);
        // could also set a maximum; see getelevationarray

        double dLatitude =
            (position2->latlonalt.latitude - position1->latlonalt.latitude)
            / (double) numSamples;
        double dLongitude =
            (position2->latlonalt.longitude - position1->latlonalt.longitude)
            / (double) numSamples;

        Coordinates position;
        //
        // This if statement is added to return the same result
        // for the given two end-points regardless of the direction
        //
        if (position1->latlonalt.latitude >= position2->latlonalt.latitude)
        {
            position.latlonalt.latitude = position1->latlonalt.latitude;
            position.latlonalt.longitude = position1->latlonalt.longitude;
        }
        else
        {
            position.latlonalt.latitude = position2->latlonalt.latitude;
            position.latlonalt.longitude = position2->latlonalt.longitude;
            dLatitude = -dLatitude;
            dLongitude = -dLongitude;
        }

        distance = 0;
        TERRAIN_SetToGroundLevel(m_terrainData, &position);
        double stepDistance;
        Coordinates oldPosition;
        int i;
        for (i = 0; i < numSamples; i++)
        {
            oldPosition.latlonalt.latitude = position.latlonalt.latitude;
            oldPosition.latlonalt.longitude = position.latlonalt.longitude;
            oldPosition.latlonalt.altitude = position.latlonalt.altitude;

            position.latlonalt.latitude += dLatitude;
            position.latlonalt.longitude += dLongitude;

            TERRAIN_SetToGroundLevel(m_terrainData, &position);
            COORD_CalcDistance(
                LATLONALT,
                &position,
                &oldPosition,
                &stepDistance);

            distance += stepDistance;
        }
    }
    else
    {
        ERROR_ReportWarning(
            "calcDistanceOverTerrain: "
            "Coordinate system must be LATLONALT "
            "and there must be terrain data. "
            "Returned direct distance.\n");
    }
    return distance;
}

void QualNetUrbanTerrainData::roadGiveIntermediatePosition(
    Coordinates* const position,
    RoadSegmentID ID,
    IntersectionID startIntersection,
    double traveling_distance)
{
    double accumulated_distance = 0;
    RoadSegment* rs = getRoadSegment(ID);
    double distance = 0;
    int i;

    if (traveling_distance >= rs->length)
    {
        ERROR_ReportError("ENVIRONMENT: tried to travel farther down "
                          "road than length of road\n");
    }

    if (rs->firstIntersection == startIntersection)
    {
        for (i = 0; i <= rs->num_vertices - 2; i++)
        {
            COORD_CalcDistance(m_terrainData->getCoordinateSystem(),
                               &(rs->vertices[i]),
                               &(rs->vertices[i + 1]),
                               &distance);
            if (accumulated_distance + distance <= traveling_distance)
            {
                accumulated_distance += distance;
            }
            else
            {
                break;
            }
        }

        if (i == rs->num_vertices -  1)
        {
            ERROR_ReportError("ENVIRONMENT: tried to travel farther down "
                              "road than length of road\n");
        }

        double fraction
        = (traveling_distance - accumulated_distance) / distance;

        position->common.c1 =
            rs->vertices[i].common.c1
            + (rs->vertices[i + 1].common.c1 - rs->vertices[i].common.c1)
            * fraction;

        position->common.c2 =
            rs->vertices[i].common.c2
            + (rs->vertices[i + 1].common.c2 - rs->vertices[i].common.c2)
            * fraction;

        position->common.c3 =
            rs->vertices[i].common.c3
            + (rs->vertices[i + 1].common.c3 - rs->vertices[i].common.c3)
            * fraction;

        //TERRAIN_SetToGroundLevel(m_terrainData, position);
    }
    else
    {
        for (i = 0; i <= rs->num_vertices - 2; i++)
        {
            COORD_CalcDistance(m_terrainData->getCoordinateSystem(),
                               &(rs->vertices[rs->num_vertices - i - 1]),
                               &(rs->vertices[rs->num_vertices - i - 2]),
                               &distance);
            if (accumulated_distance + distance <= traveling_distance)
            {
                accumulated_distance += distance;
            }
            else
            {
                break;
            }
        }

        if (i == rs->num_vertices - 1)
        {
            ERROR_ReportError("ENVIRONMENT: tried to travel farther down "
                              "road than length of road\n");
        }

        double fraction
        = (traveling_distance - accumulated_distance) / distance;

        position->common.c1 =
            rs->vertices[rs->num_vertices - i - 1].common.c1
            + (rs->vertices[rs->num_vertices - i - 2].common.c1
               - rs->vertices[rs->num_vertices - i - 1].common.c1)
            * fraction;

        position->common.c2 =
            rs->vertices[rs->num_vertices - i - 1].common.c2
            + (rs->vertices[rs->num_vertices - i - 2].common.c2
               - rs->vertices[rs->num_vertices - i - 1].common.c2)
            * fraction;

        position->common.c3 =
            rs->vertices[rs->num_vertices - i - 1].common.c3
            + (rs->vertices[rs->num_vertices - i - 2].common.c3
               - rs->vertices[rs->num_vertices - i - 1].common.c3)
            * fraction;

    }
}

std::string* QualNetUrbanTerrainData::fileList(NodeInput *nodeInput)
{
    //Array of strings containing the files names of all the XML
    //files to be parsed
    BOOL wasFound;

    std::string* response = new std::string("");

    int numFiles = 0;

    char buf[MAX_STRING_LENGTH];

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "TERRAIN-FEATURES-SOURCE",
        &wasFound,
        buf);

    if (!wasFound) {
        return NULL;
    }
    else {
        *response += " ";
        *response += buf;

        if (strcmp(buf, "FILE") == 0) {
            IO_ReadString(
                ANY_NODEID,
                ANY_ADDRESS,
                nodeInput,
                "TERRAIN-FEATURES-FILELIST",
                &wasFound,
                buf);

            if (wasFound)
            {
                delete response;
                return new std::string(""); // this isn't supported currently
            }
            else
            {
                while (TRUE) {
                    BOOL fallBackToGlobal = FALSE;
                    if (numFiles == 0) { fallBackToGlobal = TRUE; }

                    IO_ReadStringInstance(
                        ANY_NODEID,
                        ANY_ADDRESS,
                        nodeInput,
                        "TERRAIN-FEATURES-FILENAME",
                        numFiles,
                        fallBackToGlobal,
                        &wasFound,
                        buf);

                    if (!wasFound) {
                        break;
                    }
                    numFiles++;
                    *response += " ";
                    *response += buf;
                }
            }
        }
#ifdef CTDB7_INTERFACE
        else if (strcmp(buf, "CTDB7") == 0) {
            // this isn't supported currently
        }
#endif // CTDB7_INTERFACE
#ifdef CTDB8_INTERFACE
        else if (strcmp(buf, "CTDB8") == 0) {
            // this isn't supported currently
        }
#endif // CTDB8_INTERFACE
//#ifdef ESRI_SHP_INTERFACE
        else if (strcmp(buf, "SHAPEFILE") == 0) {

            char shapefilePath[MAX_STRING_LENGTH];
            char shapefileType[MAX_STRING_LENGTH];
            char defaultMsmtUnit[MAX_STRING_LENGTH];
            char defaultHeight[MAX_STRING_LENGTH];
            char defaultFoliageDensity[MAX_STRING_LENGTH];
            char dbfMsmtUnit[MAX_STRING_LENGTH];
            char dbfHeightTag[MAX_STRING_LENGTH];
            char dbfDensityTag[MAX_STRING_LENGTH];

            char thisFile[MAX_STRING_LENGTH];

            while (TRUE) {
                BOOL fallBackToGlobal = FALSE;
                if (numFiles == 0) { fallBackToGlobal = TRUE; }

                // SHAPEFILE-PATH is required
                IO_ReadStringInstance(
                    ANY_NODEID,
                    ANY_ADDRESS,
                    nodeInput,
                    "SHAPEFILE-PATH",
                    numFiles,
                    fallBackToGlobal,
                    &wasFound,
                    shapefilePath);

                if (!wasFound)
                {
                    break;
                }

                IO_ReadStringInstance(
                    ANY_NODEID,
                    ANY_ADDRESS,
                    nodeInput,
                    "SHAPEFILE-DEFAULT-SHAPE-TYPE",
                    numFiles,
                    TRUE,
                    &wasFound,
                    shapefileType);

                if (!wasFound)
                {
                    strcpy(shapefileType, "BUILDING");
                }

                IO_ReadStringInstance(
                    ANY_NODEID,
                    ANY_ADDRESS,
                    nodeInput,
                    "SHAPEFILE-DEFAULT-MSMT-UNIT",
                    numFiles,
                    TRUE,
                    &wasFound,
                    defaultMsmtUnit);

                if (!wasFound)
                {
                    strcpy(defaultMsmtUnit, "FEET");
                }

                IO_ReadStringInstance(
                    ANY_NODEID,
                    ANY_ADDRESS,
                    nodeInput,
                    "SHAPEFILE-DBF-FILE-MSMT-UNIT",
                    numFiles,
                    TRUE,
                    &wasFound,
                    dbfMsmtUnit);

                if (!wasFound)
                {
                    strcpy(dbfMsmtUnit, "FEET");
                }

                if (!strcmp(shapefileType, "BUILDING")) {
                    IO_ReadStringInstance(
                        ANY_NODEID,
                        ANY_ADDRESS,
                        nodeInput,
                        "SHAPEFILE-DEFAULT-BLDG-HEIGHT",
                        numFiles,
                        TRUE,
                        &wasFound,
                        defaultHeight);

                    if (!wasFound)
                    {
                        strcpy(defaultHeight, "35");
                    }

                    IO_ReadStringInstance(
                        ANY_NODEID,
                        ANY_ADDRESS,
                        nodeInput,
                        "SHAPEFILE-DBF-BLDG-HEIGHT-TAG-NAME",
                        numFiles,
                        TRUE,
                        &wasFound,
                        dbfHeightTag);

                    if (!wasFound)
                    {
                        strcpy(dbfHeightTag, "LV");
                    }

                    sprintf(thisFile, " \"%s\" %s %s %s %s %s",
                            shapefilePath,
                            shapefileType,
                            defaultMsmtUnit,
                            defaultHeight,
                            dbfMsmtUnit,
                            dbfHeightTag);
                    *response += thisFile;
                }
                else { // FOLIAGE
                    IO_ReadStringInstance(
                        ANY_NODEID,
                        ANY_ADDRESS,
                        nodeInput,
                        "SHAPEFILE-DEFAULT-FOLIAGE-HEIGHT",
                        numFiles,
                        TRUE,
                        &wasFound,
                        defaultHeight);

                    if (!wasFound)
                    {
                        strcpy(defaultHeight, "35");
                    }

                    IO_ReadStringInstance(
                        ANY_NODEID,
                        ANY_ADDRESS,
                        nodeInput,
                        "SHAPEFILE-DEFAULT-FOLIAGE-DENSITY",
                        numFiles,
                        TRUE,
                        &wasFound,
                        defaultFoliageDensity);

                    if (!wasFound)
                    {
                        strcpy(defaultFoliageDensity, "0.15");
                    }

                    IO_ReadStringInstance(
                        ANY_NODEID,
                        ANY_ADDRESS,
                        nodeInput,
                        "SHAPEFILE-DBF-FOLIAGE-HEIGHT-TAG-NAME",
                        numFiles,
                        TRUE,
                        &wasFound,
                        dbfHeightTag);

                    if (!wasFound)
                    {
                        strcpy(dbfHeightTag, "LV");
                    }

                    IO_ReadStringInstance(
                        ANY_NODEID,
                        ANY_ADDRESS,
                        nodeInput,
                        "SHAPEFILE-DBF-FOLIAGE-DENSITY-TAG-NAME",
                        numFiles,
                        TRUE,
                        &wasFound,
                        dbfDensityTag);


                    if (!wasFound)
                    {
                        strcpy(dbfDensityTag, "DENSITY");
                    }

                    sprintf(thisFile, " \"%s\" %s %s %s %s %s %s %s",
                            shapefilePath,
                            shapefileType,
                            defaultMsmtUnit,
                            defaultHeight,
                            dbfMsmtUnit,
                            dbfHeightTag,
                            defaultFoliageDensity,
                            dbfDensityTag);

                    *response += thisFile;
                }
                numFiles++;
            }

        }
//#endif // ESRI_SHP_INTERFACE
        else {
            ERROR_ReportError("Unknown TERRAIN-FEATURES-SOURCE\n");
        }
    }

    return response;
}

void QualNetUrbanTerrainData::groupSignals()
{
    int i, j, n;
    double cosine;

    for (n = 0; n < m_numIntersections; n++)
    {
        if (intersectionHasTrafficSignal(n) == true)
        {
            const Coordinates* intersectionLoc =
                intersectionLocation(n);

            const IntersectionID* neighbors;
            int numNeighbors;
            neighboringIntersections(n, &neighbors, &numNeighbors);

            for (i = 0; i < numNeighbors; i++)
            {
                m_intersections[n].signalGroups[i] = -1;
            }

            for (i = 0; i < numNeighbors; i++)
            {
                if (m_intersections[n].signalGroups[i] == -1)
                {
                    m_intersections[n].signalGroups[i] = i;
                }

                const Coordinates* firstNeighborLocation =
                    intersectionLocation(neighbors[i]);

                for (j = i + 1; j < numNeighbors; j++)
                {
                    if (m_intersections[n].signalGroups[j] == -1)
                    {
                        const Coordinates* secondNeighborLocation =
                            intersectionLocation(neighbors[j]);
                        cosine = cosineOfVectors(
                                     m_terrainData->getCoordinateSystem(),
                                     intersectionLoc,
                                     firstNeighborLocation,
                                     secondNeighborLocation);

                        if (cosine < -sqrt(3.0) * 0.5)
                        {
                            m_intersections[n].signalGroups[j] = m_intersections[n].signalGroups[i];
                        }
                    }
                }
            }

        }
    }
}


double QualNetUrbanTerrainData::calculateBuildingHeight(const Building* building)
{
    double tmp;

    FeatureFace* face;
    int i, j;

    double buildingHeight =
        fabs(building->faces[0].vertices[0].common.c3
             - building->faces[0].vertices[building->faces[0].num_vertices-1].common.c3);

    for (i = 0; i < building->num_faces; i++)
    {
        face = &building->faces[i];

        tmp = fabs(face->vertices[0].common.c3
                   - face->vertices[face->num_vertices-1].common.c3);

        if (tmp > buildingHeight)
        {
            buildingHeight = tmp;
        }

        for (j = 1; j < face->num_vertices; j++)
        {
            tmp = fabs(face->vertices[j].common.c3
                       - face->vertices[j-1].common.c3);
            if (tmp > buildingHeight)
            {
                buildingHeight = tmp;
            }
        }
    }
    return buildingHeight;
}


void QualNetUrbanTerrainData::addHeightInformation(Building* building,
        float* minBuildingHeight,
        float* maxBuildingHeight,
        float* totalBuildingHeight)
{
    building->height = calculateBuildingHeight(building);

    if (building->height < *minBuildingHeight)
    {
        *minBuildingHeight = (float)building->height;
    }
    if (building->height > *maxBuildingHeight)
    {
        *maxBuildingHeight = (float)building->height;
    }
    *totalBuildingHeight += (float)building->height;
}


void QualNetUrbanTerrainData::convertBuildingCoordinates(
    Building* building,
    CoordinateRepresentationType type)
{
    FeatureFace* face;
    int i;
    int j;

    Coordinates tmp;

    for (i = 0; i < building->num_faces; i++)
    {
        face = &building->faces[i];

        for (j = 0; j < face->num_vertices; j++)
        {
            memcpy(&tmp, &(face->vertices[j]), sizeof(Coordinates));
            tmp.type = GEODETIC;
            COORD_ChangeCoordinateSystem(&tmp,
                                         type,
                                         &(face->vertices[j]));
        }
    }
}

void QualNetUrbanTerrainData::setTerrainFeaturesToGround(Building* features,
        int numFeatures)
{
    Coordinates featurePositionGround;
    size_t size = sizeof(featurePositionGround);
    int i, j, k;
    for (i = 0; i < numFeatures; i++)
    {
        // Initialize ground elevation to first vertex of first face of
        // obstruction.
        Building* feature = &features[i];
        memcpy(&featurePositionGround, &feature->faces[0].vertices[0],size);
        TERRAIN_SetToGroundLevel(m_terrainData, &featurePositionGround);
        double groundElevation = featurePositionGround.common.c3;

        // For best results (no floating buildings/foliage), obtain minimum
        // ground elevation across all vertices for a given obstruction.
        for (j = 0; j < feature->num_faces; ++j)
        {
            for (k = 0; k < feature->faces[j].num_vertices; ++k)
            {
                memcpy(&featurePositionGround,
                    &feature->faces[j].vertices[k], size);
                TERRAIN_SetToGroundLevel(
                    m_terrainData, &featurePositionGround);

                if (featurePositionGround.common.c3 < groundElevation)
                {
                    groundElevation = featurePositionGround.common.c3;
                }
            }
        }

        if (NODEBUG) {
            printf("Feature %u ground %.1f\n", i, groundElevation);
        }

        // Update all feature vertices with new height.
        for (j = 0; j < feature->num_faces; j++)
        {
            FeatureFace* face = &feature->faces[j];

            for (k = 0; k < face->num_vertices; k++)
            {
                face->vertices[k].common.c3 += groundElevation;
            }
        }//for//
    }//for//
}

void QualNetUrbanTerrainData::returnIntersectionBuildings(
    BgiResult& result,
    Building* obstructions,
    BgLinestring& path,
    int* const numResults,
    BuildingID** const buildings,
    Coordinates (** const intersections)[2],
    FaceIndex (** const faces)[2])
{
    int i;
    int maxResults = result.size();

    bool isOverlap;
    Coordinates intersection1;
    Coordinates intersection2;
    Coordinates source;
    source.type = GEOCENTRIC_CARTESIAN;
    Coordinates dest;
    dest.type = GEOCENTRIC_CARTESIAN;
 
    FaceIndex face1;
    FaceIndex face2;

    *numResults = 0;

    if (buildings)
    {
        (*buildings) = (BuildingID*) MEM_malloc(maxResults * sizeof(BuildingID));
    }
    if (intersections)
    {
        (*intersections) = (Coordinates(*)[2])
            MEM_malloc(maxResults * sizeof(Coordinates) * 2);
    }
    if (faces)
    {
        (*faces) = (FaceIndex(*)[2])
            MEM_malloc(maxResults * sizeof(FaceIndex) * 2);
    }

    if (NODEBUG) {
        std::cout << "returnIntersectionBuildings checking path:"
                  << bg::dsv(path) << std::endl;
    }

    for (i = 0; i < maxResults; i++)
    {
        // First checks the LoS. If no overlap, check each segment in the 
        // path, but indicate not to test for the endpoints of the path
        // segments being inside a building.
        // (Note: path is assured to have at least two points.)
        isOverlap = false;
        BgLinestring::iterator ip = path.begin();
        source.cartesian.x = ip->get<0>();
        source.cartesian.y = ip->get<1>();
#ifdef BG_USE_3D
        source.cartesian.z = ip->get<2>();
#endif
        // The first point in the path is the TX point.
        // If there are only two points, then the path is a simple LoS.
        // If there are more, then the third point is the RX point.
        ip++;
        if (path.size() > 2)
        {
            ip++; // get 3rd coordinate from Fresnel shape
        }

        dest.cartesian.x = ip->get<0>();
        dest.cartesian.y = ip->get<1>();
#ifdef BG_USE_3D
        dest.cartesian.z = ip->get<2>();
#endif

        // Does the LoS intersect the obstruction?
        isOverlap = calculateOverlap(
            m_terrainData->getCoordinateSystem(),
            source,
            dest,
            &(obstructions[result[i].second]),
            true, // check inside/outside obstruction
            intersection1,
            intersection2,
            face1,
            face2);

        if (!isOverlap && path.size() > 2)
        {
            // The LoS did not intersect the obstruction. If the path is a
            // Fresnel shape, determine if any of the sub-segments intersect
            // the obstruction, indicating a partial obstruction of the 
            // Fresnel zone 1.

            // Reset the iterator to the second point in the path
            ip = path.begin();
            ++ip;

            while (!isOverlap && ip != path.end())
            {
                dest.cartesian.x = ip->get<0>();
                dest.cartesian.y = ip->get<1>();
    #ifdef BG_USE_3D
                dest.cartesian.z = ip->get<2>();
    #endif
                isOverlap = calculateOverlap(
                    m_terrainData->getCoordinateSystem(),
                    source,
                    dest,
                    &(obstructions[result[i].second]),
                    false, // don't check inside/outside for sub-segments of LoS
                    intersection1,
                    intersection2,
                    face1,
                    face2);
                source = dest;
                ip++;
            }
        } // path.size() > 2

        if (isOverlap)
        {
            if (NODEBUG)
            {
                printf("Overlap: B%03d SEG:(%.1f,%.1f,%.1f)--(%.1f,%.1f,%.1f) "
                    "F%03d @ (%.1f,%.1f,%.1f) F%03d @ (%.1f,%.1f,%.1f)\n",
                    result[i].second,
                    source.cartesian.x, source.cartesian.y, source.cartesian.z, 
                    dest.cartesian.x, dest.cartesian.y, dest.cartesian.z, 
                    face1, intersection1.cartesian.x, intersection1.cartesian.y, 
                    intersection1.cartesian.z, 
                    face2, intersection2.cartesian.x, intersection2.cartesian.y, 
                    intersection2.cartesian.z);
            }

            // TBD, results are in GEOCENTRIC_CARTESIAN, so 
            // why not leave them in GCC?
            if (m_terrainData->getCoordinateSystem() == LATLONALT)
            {
                Coordinates save1;
                Coordinates save2;

                COORD_ChangeCoordinateSystem(
                    &intersection1, GEODETIC, &save1);
                COORD_ChangeCoordinateSystem(
                    &intersection2, GEODETIC, &save2);

                memcpy(&intersection1, &save1, sizeof(Coordinates));
                memcpy(&intersection2, &save2, sizeof(Coordinates));
            }

            // out of space 
            if (*numResults == maxResults)
            {
                if (buildings)
                {
                    BuildingID* tmpBuildings = (*buildings);

                    (*buildings) = (BuildingID*)
                        MEM_malloc(2 * maxResults * sizeof(BuildingID));

                    memcpy((*buildings),
                           tmpBuildings, maxResults * sizeof(BuildingID));

                    MEM_free(tmpBuildings);
                }

                if (intersections)
                {
                    Coordinates (* tmpIntersections)[2] = (*intersections);

                    (*intersections) = (Coordinates(*)[2])
                        MEM_malloc(2 * maxResults * sizeof(Coordinates) * 2);

                    memcpy((*intersections),
                           tmpIntersections, maxResults * sizeof(Coordinates) * 2);

                    MEM_free(tmpIntersections);
                }

                if (faces)
                {
                    FaceIndex (* tmpFaces)[2] = (*faces);

                    (*faces) = (FaceIndex(*)[2])
                        MEM_malloc(2 * maxResults * sizeof(FaceIndex) * 2);

                    memcpy((*faces), tmpFaces, maxResults * sizeof(FaceIndex) * 2);

                    MEM_free(tmpFaces);
                }
                maxResults *= 2;
            }

            if (buildings)
            {
                (*buildings)[*numResults] = result[i].second; // Store the building index
            }

            if (intersections)
            {
                memcpy(&((*intersections)[*numResults][0]),
                       &intersection1,
                       sizeof(Coordinates));
                memcpy(&((*intersections)[*numResults][1]),
                       &intersection2,
                       sizeof(Coordinates));
            }

            if (faces)
            {
                (*faces)[*numResults][0] = face1;
                (*faces)[*numResults][1] = face2;
            }

            (*numResults)++;
        }
    }
}
