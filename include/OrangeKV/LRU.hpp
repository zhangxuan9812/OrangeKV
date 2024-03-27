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
struct LRUNode {
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
class LRUCache {
private:
    size_t capacity_; // The maximum capacity of the cache
    size_t usage_; // The total charge of the cache
    std::list<LRUNode<KeyType, ValueType>*> lruList; // The list of nodes that are not in use
    std::list<LRUNode<KeyType, ValueType>*> inUseList; // The list of nodes that are in use
    std::unordered_map<KeyType, typename std::list<LRUNode<KeyType, ValueType>*>::iterator, OrangeKV::HashBKDR> lruMap; // The map of keys to nodes
    LockType locker; // The locker for thread safety
public:
    LRUCache(); // Constructor
    ~LRUCache(); // Destructor
    Handle* insert(const KeyType& key, uint32_t hash, ValueType* value, size_t charge, void (*deleter)(const KeyType& key, ValueType* value)); // Insert a new node into the cache
    Handle* lookUp(const KeyType& key, uint32_t hash); // Look up a node in the cache
    void release(Handle* handle); // Release a node from the cache
    void erase(const KeyType& key, uint32_t hash); // Erase a node from the cache
    void prune(); // Prune the cache
    void setCapacity(size_t capacity) { // Set the maximum capacity of the cache
        capacity_ = capacity;
    }
    size_t capacity() const { // Get the maximum capacity of the cache
        return capacity_;
    }
    size_t totalCharge() const { // Get the total charge of the cache
        return usage_;
    }
private:
    void lruRemove(std::list<LRUNode<KeyType, ValueType>*>& list, LRUNode<KeyType, ValueType>* node); // Remove a node from a list
    void lruAppend(std::list<LRUNode<KeyType, ValueType>*>& list, LRUNode<KeyType, ValueType>* node); // Append a node to a list
    void ref(LRUNode<KeyType, ValueType>* node); // Increase the reference count of a node
    void unref(LRUNode<KeyType, ValueType>* node); // Decrease the reference count of a node
    bool finishErase(LRUNode<KeyType, ValueType>* node); // Finish erasing a node
};



template<typename KeyType, typename ValueType, typename LockType>
LRUCache<KeyType, ValueType, LockType>::LRUCache() : capacity_(0), usage_(0) {}

template<typename KeyType, typename ValueType, typename LockType>
LRUCache<KeyType, ValueType, LockType>::~LRUCache() {
    // Release all handles
    static_assert(inUseList.empty(), "inUseList.empty()");  // The in-use list must be empty
    for (auto it = lruList.begin(); it != lruList.end(); ++it) {
        LRUNode<KeyType, ValueType>* node = *it;
        node->inCache = false;
        static_assert(node->refs == 1, "node->refs == 1"); // The reference count of the node must be 1
        unref(node);
    }
}

template<typename KeyType, typename ValueType, typename LockType>
Handle* LRUCache<KeyType, ValueType, LockType>::insert(const KeyType& key, uint32_t hash, ValueType* value, size_t charge, void (*deleter)(const KeyType& key, ValueType* value)) {
    std::lock_guard<LockType> lock(locker);
    // Create a new node
    auto newNode = new LRUNode<KeyType, ValueType>();
    newNode->deleter = deleter;
    newNode->key = new KeyType(key);
    newNode->value = value;
    newNode->charge = charge;
    newNode->keyLength = key.size();
    newNode->inCache = true; // The node is in the cache
    newNode->hash = hash;
    newNode->refs = 1;
    std::memcpy(newNode->keyData, key.data(), key.size()); // Copy the key data to the node
    // Check if the key already exists in the cache
    //auto it = lruMap.find(newNode->key);
    auto it  = lruMap.find(*(newNode->key));
    if (it != lruMap.end()) {
        // Key already exists, update the value and move the node to the front of the LRU list
        LRUNode<KeyType, ValueType>* node = *(it->second);  // Get the node
        newNode->refs++; // Increase the reference count of the new node

        lruAppend(inUseList, newNode); // Append the new node to the in-use list
        lruMap[*(newNode->key)] = inUseList.begin(); // Update the key in the LRU map
        finishErase(node); // Finish erasing the old node
        usage_ += charge;
    }
    else {
        // Key does not exist, add the node to the in-use list and the LRU map
        newNode->refs++; // Increase the reference count of the new node
        lruAppend(inUseList, newNode); // Append the node to the in-use list
        lruMap.emplace(*(newNode->key), inUseList.begin()); // Add the key to the LRU map
        usage_ += charge; // Update the cache usage
    }
    // Prune the cache if the usage exceeds the capacity
    while (usage_ > capacity_ && !lruList.empty()) {
        LRUNode<KeyType, ValueType>* node = lruList.back(); // Get the last node in the LRU list
        static_assert(node->refs == 1, "node->refs == 1"); // The reference count of the node must be 1
        lruMap.erase(*(node->key)); // Erase the key from the LRU map
        finishErase(node); // Finish erasing the node
    }
    return reinterpret_cast<Handle*>(newNode); // Return the handle
}


template<typename KeyType, typename ValueType, typename LockType>
Handle* LRUCache<KeyType, ValueType, LockType>::lookUp(const KeyType& key, uint32_t hash) {
    std::lock_guard<LockType> lock(locker);
    
    // Check if the key exists in the cache
    auto it = lruMap.find(key);
    if (it != lruMap.end()) {
        LRUNode<KeyType, ValueType>* node = *(it->second); // Get the node
        if (node->hash == hash) {
            ref(node); // Increase the reference count of the node
            return reinterpret_cast<Handle*>(node);
        }
    }
    else {
        return reinterpret_cast<Handle*>(nullptr);
    }
}

template<typename KeyType, typename ValueType, typename LockType>
void LRUCache<KeyType, ValueType, LockType>::release(Handle* handle) {
    std::lock_guard<LockType> lock(locker);
    
    // Release the handle
    LRUNode<KeyType, ValueType>* node = reinterpret_cast<LRUNode<KeyType, ValueType>*>(handle);
    unref(node);
}

template<typename KeyType, typename ValueType, typename LockType>
void LRUCache<KeyType, ValueType, LockType>::erase(const KeyType& key, uint32_t hash) {
    std::lock_guard<LockType> lock(locker);
    
    // Check if the key exists in the cache
    auto it = lruMap.find(key);
    if (it != lruMap.end()) {
        LRUNode<KeyType, ValueType>* node = *(it->second);
        if (node->hash == hash) {
            lruMap.erase(it);
            finishErase(node);
        }
    }
}

template<typename KeyType, typename ValueType, typename LockType>
void LRUCache<KeyType, ValueType, LockType>::prune() {
    while (!lruList.empty()) {
        LRUNode<KeyType, ValueType>* node = lruList.back();
        lruMap.erase(*(node->key));
        finishErase(node);
    }
}

template<typename KeyType, typename ValueType, typename LockType>
void LRUCache<KeyType, ValueType, LockType>::lruRemove(std::list<LRUNode<KeyType, ValueType>*>& list, LRUNode<KeyType, ValueType>* node) {
    list.erase(node);
}

template<typename KeyType, typename ValueType, typename LockType>
void LRUCache<KeyType, ValueType, LockType>::lruAppend(std::list<LRUNode<KeyType, ValueType>*>& list, LRUNode<KeyType, ValueType>* node) {
    list.push_front(node);
}

template<typename KeyType, typename ValueType, typename LockType>
void LRUCache<KeyType, ValueType, LockType>::ref(LRUNode<KeyType, ValueType>* node) {
    // Increase the reference count of the node
    if (node->refs == 1 && node->inCache) { // Move the node to the front of the in-use list
        lruRemove(lruList, node); // Remove the node from the LRU list
        lruAppend(inUseList, node); // Append the node to the in-use list
    }
    node->refs++; // Increase the reference count
}


template<typename KeyType, typename ValueType, typename LockType>
void LRUCache<KeyType, ValueType, LockType>::unref(LRUNode<KeyType, ValueType>* node) { 
    node->refs--;
    if (node->refs == 0) { // Erase the node if the reference count is 0
        (*node->deleter)(*(node->key), node->value);
        free(node);
    }
    else if (node->refs == 1 && node->inCache == true) { 
        lruRemove(inUseList, node);
        lruAppend(lruList, node);
    }
}

template<typename KeyType, typename ValueType, typename LockType>
bool LRUCache<KeyType, ValueType, LockType>::finishErase(LRUNode<KeyType, ValueType>* node) {
    if (node != nullptr) {
        static_assert(node->inCache == true, "node->inCache == true");
        if (node->refs == 1) {
            lruRemove(lruList, node);
        }
        else if (node->refs >= 2) { //Can't move the node to the lruList, because node->inCache will be set to false
            lruRemove(inUseList, node);
        }
        node->inCache = false;
        usage_ -= node->charge;
        unref(node);    
    }
    return node != nullptr;
}