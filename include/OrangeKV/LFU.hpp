#include <cstdint>
#include <cstddef>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <memory> 
#include "utility/hash.hpp"


//The type of node
template<typename KeyType, typename ValueType>
struct LFUNode {
    void (*deleter)(const KeyType& key, ValueType* value);
    KeyType* key;
    ValueType* value;
    size_t charge;
    size_t keyLength;
    bool inCache;
    uint32_t hash;
    uint32_t refs;
    char keyData[1];
};

struct Handle{};


template<typename KeyType, typename ValueType, typename LockType>
class LFUCache {
private:
    size_t capacity_; // The maximum capacity of the cache
    size_t usage_; // The total charge of the cache
    std::list<LFUNode<KeyType, ValueType>> lruList; // The list of nodes that are not in use
    std::list<LFUNode<KeyType, ValueType>> inUseList; // The list of nodes that are in use
    std::unordered_map<KeyType, typename std::list<LFUNode<KeyType, ValueType>*>::iterator, OrangeKV::HashBKDR> lruMap; // The map of keys to nodes
    LockType locker; // The locker for thread safety
public:
    LFUCache(); // Constructor
    ~LFUCache(); // Destructor
    Handle* insert(const KeyType& key, uint32_t hash, ValueType* value, size_t charge, void (*deleter)(const KeyType& key, ValueType* value)); // Insert a new node into the cache
    Handle* lookUp(const KeyType& key, uint32_t hash); // Look up a node in the cache
    void release(Handle* handle); // Release a node from the cache
    void erase(const KeyType& key, uint32_t hash); // Erase a node from the cache
    void prune(); // Prune the cache
    size_t capacity() const { // Get the maximum capacity of the cache
        return capacity_;
    }
    size_t totalCharge() const { // Get the total charge of the cache
        return usage_;
    }
private:
    void lfuRemove(std::list<LFUNode<KeyType, ValueType>*>& list, LFUNode<KeyType, ValueType>* node); // Remove a node from a list
    void lfuAppend(std::list<LFUNode<KeyType, ValueType>*>& list, LFUNode<KeyType, ValueType>* node); // Append a node to a list
    void ref(LFUNode<KeyType, ValueType>* node); // Increase the reference count of a node
    void unref(LFUNode<KeyType, ValueType>* node); // Decrease the reference count of a node
    bool finishErase(LFUNode<KeyType, ValueType>* node); // Finish erasing a node
};


#include <iostream>
