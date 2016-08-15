#ifndef _CACHE_H_
#define _CACHE_H_

#include <list>
#include "unordered_map_config.h"

/// This is the initial max size of the cache when the default constructor
/// is used. The size can be changed dynamically.
const size_t defaultCacheSize = 256;


/// \brief LRU (Least Recently Used) Cache template class
///
/// Template class for a LRU cache. The maximum cache size can be changed 
/// dynamically. The cache maintains a hash map of values by key type. It
/// uses a use-list to keep track of usage. The LRU (least recently used) 
/// key value is at the front of the list. The MRU (most recently used) 
/// key value is at the back of the list. The value-map contains an iterator
/// into the use-list. When a key lookup in the map is successful, the entry
/// is moved to the end of the use-list. When the cache is full, the key
/// value from the front of the list (the LRU) is evicted to make room.
///
/// \tparam _Key_t type of the lookup key for the cache
/// \tparam _Val_t type of the value that is cached for the key
/// \tparam _Hasher class used to provide hashed values from _Key_t
///
/// To replace the _Hasher, create a template class matching std::tr1::hash.
/*
    struct MyHash : public std::unary_function<_Key_t, size_t>
    {
        size_t operator()(const _Key_t& v) const
        {
            return hash_algorithm(v); // Insert actual hash algorithm here
        }
    };
*/
/// \tparam _Keyeq class that provides a bool operator() to test equality
///
/// To replace _Keyeq, create an equivalent class to std::equal_to for the
/// key type. Alternately, use the default std::equal_to and provide the
/// an operator== definition for the key type.
/*
    struct MyEqualTo : std::binary_function <_Key_t, _Key_t, bool> {
      bool operator() (const _Key_t& x, const _Key_t& y) const {return x==y;}
    };
*/
template<typename _Key_t,
         typename _Val_t,
         class _Hasher = UNORDERED_MAP_HASH<_Key_t>,
         class _Keyeq = UNORDERED_MAP_EQUAL<_Key_t> >
class Cache
{
public:
    typedef _Key_t KeyType;
    typedef _Val_t ValueType;
    typedef std::list<_Key_t> UseList;
    typedef UNORDERED_MAP<
        _Key_t, 
        std::pair<_Val_t, typename UseList::iterator>,
        _Hasher, 
        _Keyeq> ValueMap;

    Cache() : m_max(defaultCacheSize) {}
    Cache(size_t max) : m_max(max) {}
    ~Cache() {}

    /// \brief clear the cache
    void clear() {while (evict());}

    /// \brief Looks up the key in the cache.
    ///
    /// If the key is found, moves the key to the MRU position in the
    /// use-list, outputs the value, and returns true.
    ///
    /// \param[in] key the key to seek for
    /// \param[out] val the value is set if the key is found
    /// \return true if found, indicating val has been set
    //
    bool find(const KeyType& key, ValueType& val)
    {
        typename ValueMap::iterator it = m_valueMap.find(key);
        if (it == m_valueMap.end())
        {
            // not in cache
            return false;
        }
        // found -- move the key to the end of the use list (MRU).
        m_useList.splice( 
            m_useList.end(), 
            m_useList, 
            (*it).second.second);
        // Output the value
        val = (*it).second.first;
        // Report found
        return true;
    }

    /// \brief Inserts a new key/value pair as MRU.
    ///
    /// The insert is considered to be a pair not already in the cache. 
    /// A new pair is inserted as the MRU entry. It will force an eviction 
    /// if the cache is full.
    ///
    /// \param key The key for the new pair
    /// \param val The value corresponding to the key
    /// \warning No check is done here to prevent duplicates in the cache.
    ///
    void insert(const KeyType& key, const ValueType& val)
    {
        // Make room if full
        if (getSize() >= m_max)
            evict();

        // Make this the MRU key
        typename UseList::iterator it = 
            m_useList.insert(m_useList.end(), key);

        // Store the value 
        m_valueMap.insert(std::make_pair(key, std::make_pair(val, it)));
    }

    /// \brief evicts the LRU pair from the cache.
    /// \return true if an eviction took place. false if the cache was empty.
    bool evict()
    {
        if (getSize() == 0)
            return false;
        // The front of the use-list contains the key of the LRU entry. 
        // First, find that key in the value-map.
        typename ValueMap::iterator it = m_valueMap.find(m_useList.front());
        // For performance reasons there is no:
        //     assert(it != m_valueList.end());
        // Why use a cache if screaming performance is not the goal? 
        // Remove the entry from the value map.
        m_valueMap.erase(it);
        // Complete the eviction by removing the front value from the 
        // use-list.
        m_useList.pop_front();

        return true;
    }

    size_t getSize() {return m_valueMap.size();}
    size_t getMax()  {return m_max;}
    void setMax(size_t max) 
    {
        while (getSize() > max)
        {
            evict();
        }
        m_max = max;
    }

#ifdef CACHE_DEBUG
    /// Provides access to the use-list for debugging
    typename UseList::iterator useListBegin() 
    {
        return m_useList.begin();
    }
    typename UseList::iterator useListEnd() 
    {
        return m_useList.end();
    }

    /// Provides access to the cache values for debugging
    typename ValueMap::iterator valueMapBegin() 
    {
        return m_valueMap.begin();
    }
    typename ValueMap::iterator valueMapEnd() 
    {
        return m_valueMap.end();
    }
#endif

private:
    size_t m_max;
    UseList m_useList;
    ValueMap m_valueMap;
};

#endif // _CACHE_H_
