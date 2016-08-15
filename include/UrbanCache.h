#ifndef _URBAN_CACHE_H_
#define _URBAN_CACHE_H_

/// \file This file defines and implements the custom types, functions,
/// and classes needed to use the template Cache class to contain
/// UrbanPathProperties objects indexed by line segments.

#include <utility>
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include "Cache.h"

/// Coordinate type for the Urban Cache
typedef int UrbanCacheCoord;

/// Point type for the Urban Cache
typedef std::pair<UrbanCacheCoord, UrbanCacheCoord> UrbanCachePoint;

/// \brief Key value for the Urban Terrain Cache
///
/// The key for the urban terrain cache is a line segment, consisting of a
/// pair of endpoints, each point consisting of two coordinates. For use
/// as a key value, floating point coordinates are converted to 
/// integer values.
typedef std::pair<UrbanCachePoint, UrbanCachePoint> UrbanCacheLine;

/// \brief fraction of a meter used for equality test and hash function
///
/// The granularity factor is used to convert floating point coordinates
/// to integer values. As a result, this factor determines when coordinates
/// are close enough to be treated as equal. For example, if the value
/// is 0.1, then the values >= 1.0 and < 1.1 will all be equal as far as
/// matching the cache key, a range of 10 centimeters.
const double coordinateGranularity = 0.1; // meters

/// Convert a double value to a fixed point integer coordinte.
inline UrbanCacheCoord toUrbanCacheCoord(double coord) 
{
    return static_cast<UrbanCacheCoord>(coord / coordinateGranularity);
}
 
/// \brief Equality operator for UrbanCachePoint 
///
/// This definition allows the use of std::equal_to for testing equality.
inline bool operator==(const UrbanCachePoint& lhs, const UrbanCachePoint& rhs)
{
    return (lhs.first == rhs.first) && (lhs.second == rhs.second);
}

/// \brief Equality operator for UrbanCacheLine
///
/// This definition allows the use of std::equal_to for testing equality.
inline bool operator==(const UrbanCacheLine& lhs, const UrbanCacheLine& rhs)
{
    return (lhs.first == rhs.first) && (lhs.second == rhs.second);
}

/// \brief Hasher class for the Urban Terrain Key, UrbanCacheLine
///
/// The typical characteristics of the urban terrain cartesian coordinates
/// are that they encompass an area that is perhaps up to hundred kilometers
/// in range, but are not necessarily located anywhere near to 0,0. 
///
/// The hash algorithm has to combine the four endpoints of the line segment.
/// Therefore, this implementation uses boost::hash_combine that is based on
/// boost::hash_value.
struct HashUrbanCacheLine : public std::unary_function<UrbanCacheLine, size_t>
{
    size_t operator()(const UrbanCacheLine& v) const
    {
        size_t seed = 0;
        boost::hash_combine(seed, v.first.first);
        boost::hash_combine(seed, v.first.second);
        boost::hash_combine(seed, v.second.first);
        boost::hash_combine(seed, v.second.second);
        return seed;
    }
};

/// \brief Value type stored in the cache
///
/// The cache contains QualNetUrbanPathProperties, a complex structure 
/// that is created for a given tx/rx pair. Since it is already created and 
/// non-trivial, the pointer should be stored in the cache. A boost
/// smart pointer is used to make sure that the structure will be 
/// not be deleted before its cache entry is evicted.
class UrbanPathProperties;
typedef boost::shared_ptr<UrbanPathProperties> UrbanCacheValue;

typedef
Cache<UrbanCacheLine, UrbanCacheValue, HashUrbanCacheLine> UrbanCache;

#endif // _URBAN_CACHE_H_
