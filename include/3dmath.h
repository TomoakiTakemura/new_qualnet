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

/// \defgroup Package_3D_MATH 3D_MATH

/// \file
/// \ingroup Package_3D_MATH
/// This file describes data structures and functions used to model 3D weather patterns in conjunction
/// with the Weather package.

#ifndef _QUALNET_3DMATH_H
#define _QUALNET_3DMATH_H

#include <math.h>
#include <main.h>

#define MARGIN_OF_ERROR 0.001

/// This is used to hold 3D points and vectors.  This will
/// eventually be added upon to create a robust class with
/// operator overloading.  For now we just need an x, y, z.
struct Vector3 {
public:

    Vector3() {
        x = 0.0; y = 0.0; z = 0.0;
    }

    Vector3(double X, double Y, double Z) {
        x = X; y = Y; z = Z;
    }

    Vector3 operator+(Vector3 vector) {
        return Vector3(vector.x + x, vector.y + y, vector.z + z);
    }

    Vector3 operator-(Vector3 vector) {
        return Vector3(x - vector.x, y - vector.y, z - vector.z);
    }

    Vector3 operator*(double num) {
        return Vector3(x * num, y * num, z * num);
    }

    Vector3 operator/(double num) {
        return Vector3(x / num, y / num, z / num);
    }

    void operator+=(Vector3 vector) {
        x += vector.x; y += vector.y; z += vector.z;
    }

    void operator-=(Vector3 vector) {
        x -= vector.x; y -= vector.y; z -= vector.z;
    }
    bool operator==(Vector3 vector)
    {
        return ((fabs(x - vector.x) < MARGIN_OF_ERROR) &&
                (fabs(y - vector.y) < MARGIN_OF_ERROR) &&
                (fabs(z - vector.z) < MARGIN_OF_ERROR));
    }
    double x;
    double y;
    double z;
};

/// This struture will hold information for one triangle.
struct Triangle3 {
    Vector3 sides[3];
};


/// Returns a perpendicular vector from 2 given
/// vectors by taking the cross product.
///
/// \param vector1  the first vector
/// \param vector2  the second vector
///
/// \return the cross product
Vector3 MATH_CrossProduct(Vector3 vector1,
                          Vector3 vector2);

/// Returns a vector between 2 points
///
/// \param point1  the first point
/// \param point2  the second point
///
/// \return a vector between the two points
Vector3 MATH_Vector(Vector3 point1,
                    Vector3 point2);

/// Returns the magnitude of a normal (or any other vector)
///
/// \param vector  a vector
///
/// \return the magnitude of the vector
double MATH_Magnitude(Vector3 vector);

/// Returns a normalized vector (of exactly length 1)
///
/// \param vector  a vector
///
/// \return a normalized vector
Vector3 MATH_Normalize(Vector3 vector);

/// Returns the direction the polygon is facing
///
/// \param triangle  an array of vectors representing a polygon
///
/// \return the direction vector
Vector3 MATH_Normal(Vector3 triangle[]);

/// Returns the distance the plane is from the origin
/// (0, 0, 0).  It takes the normal to the plane, along
/// with ANY point that lies on the plane (any corner)
///
/// \param vector  a vector
/// \param point  a point
///
/// \return the plane's distance from the origin (0,0,0)
double MATH_PlaneDistance(Vector3 vector,
                         Vector3 point);

/// Takes a triangle (plane) and line and returns true
/// if they intersected
///
/// \param polygon  a polygon
/// \param line  a line
/// \param normal  a normalized vector
/// \param originDistance  the distance
///
/// \return True if they intersect
BOOL MATH_IntersectedPlane(Vector3  polygon[],
                           Vector3  line[],
                           Vector3& normal,
                           double&   originDistance);

/// Returns the dot product between 2 vectors.
///
/// \param vector1  the first vector
/// \param vector2  the second vector
///
/// \return the dot product of the two vectors
double MATH_DotProduct(Vector3 vector1,
                      Vector3 vector2);

/// This returns the angle between 2 vectors
///
/// \param vector1  the first vector
/// \param vector2  the second vector
double MATH_AngleBetweenVectors(Vector3 vector1,
                                Vector3 vector2);

/// Returns an intersection point of a polygon and a line
/// (assuming intersects the plane)
///
/// \param normal  a polygon
/// \param line  a line
/// \param distance  the distance between?
Vector3 MATH_IntersectionPoint(Vector3 normal,
                               Vector3 line[],
                               double  distance);

/// Returns true if the intersection point is inside of
/// the polygon
///
/// \param intersection  an intersection point
/// \param polygon  a polygon
/// \param verticeCount  number of points in polygon
///
/// \return True if the intersection point is in the polygon
BOOL MATH_InsidePolygon(Vector3 intersection,
                        Vector3 polygon[],
                        int     verticeCount);

/// Tests collision between a line and polygon
///
/// \param polygon  a polygon
/// \param line  a line
/// \param verticeCount  number of points in polygon
///
/// \return True if the polygon and line intersect
BOOL MATH_IntersectedPolygon(Vector3 polygon[],
                             Vector3 line[],
                             int     verticeCount);

/// Returns the distance between 2 3D points
///
/// \param point1  the first point
/// \param point2  the second point
///
/// \return the distance between the two points
double MATH_Distance(Vector3 point1,
                     Vector3 point2);

/// Checks whether two lines intersect each other or not.
///
/// \param line1  the first line
/// \param line2  the second line
///
/// \return True if the lines intersect
BOOL MATH_LineIntersects(Vector3 line1[],
                         Vector3 line2[]);

/// Returns the point of intersection between two lines.
///
/// \param line1  the first line
/// \param line2  the second line
///
/// \return the intersection point
Vector3 MATH_ReturnLineToLineIntersectionPoint(Vector3 line1[],
                                               Vector3 line2[]);

/// Returns the whether the given point lies on Line or not.
///
/// \param point  the point which we are checking.
/// \param line  the line on which the point might lie.
///
/// \return TRUE if the point lies on line
BOOL MATH_IsPointOnLine(Vector3 point,
                        Vector3 line[]);


/// Converts given cartesian coordinates to Latitide and Longitude
///
/// \param x1  Specifies X value on X-Axis
/// \param y1  Specifies Y value on Y-Axis
/// \param latitude  Will store the converted latitude value
/// \param longitude  Will store the converted longitude value
void MATH_ConvertXYToLatLong(double x1,
                             double y1,
                             double *latitude,
                             double *longitude);

#endif
