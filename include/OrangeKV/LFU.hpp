#include <cstdint>
#include <cstddef>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <memory> 
#include "utility/hash.hpp"

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
    uint32_t frequency; // The frequency of the node
    char keyData[1];
};

struct Handle{};


template<typename KeyType, typename ValueType, typename LockType>
class LFUCache {
private:
    size_t capacity_; // The maximum capacity of the cache
    size_t usage_; // The total charge of the cache
    std::unordered_map<uint32_t, typename std::list<LFUNode<KeyType, ValueType>*>::iterator> frequencyList; 
    std::unordered_map<KeyType, typename std::list<LFUNode<KeyType, ValueType>*>::iterator, OrangeKV::HashBKDR> lfuMap; 
    LockType locker; // The locker for thread safety
public:
    LFUCache(); // Constructor
    ~LFUCache(); // Destructor
    Handle* insert(const KeyType& key, uint32_t hash, ValueType* value, size_t charge, void (*deleter)(const KeyType& key, ValueType* value)); 
    Handle* lookUp(const KeyType& key, uint32_t hash); 
    void release(Handle* handle); 
    void erase(const KeyType& key, uint32_t hash); 
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
    void lfuRemove(std::list<LFUNode<KeyType, ValueType>*>& list, LFUNode<KeyType, ValueType>* node); 
    void lfuAppend(std::list<LFUNode<KeyType, ValueType>*>& list, LFUNode<KeyType, ValueType>* node); 
    void ref(LFUNode<KeyType, ValueType>* node); 
    void unref(LFUNode<KeyType, ValueType>* node);
    bool finishErase(LFUNode<KeyType, ValueType>* node); 
};


template<typename KeyType, typename ValueType, typename LockType>
LFUCache<KeyType, ValueType, LockType>::LFUCache() : capacity_(0), usage_(0) {}

template<typename KeyType, typename ValueType, typename LockType>
LFUCache<KeyType, ValueType, LockType>::~LFUCache() {
    // Clean up the cache
    for (auto it = frequencyList.begin(); it != frequencyList.end(); ++it) {
        for (auto node : it->second) {
            delete node;
        }
    }
}

template<typename KeyType, typename ValueType, typename LockType>
Handle* LFUCache<KeyType, ValueType, LockType>::insert(const KeyType& key, uint32_t hash, ValueType* value, size_t charge, void (*deleter)(const KeyType& key, ValueType* value)) {
    std::lock_guard<LockType> lock(locker);
    size_t keyLength = sizeof(KeyType);
    size_t nodeSize = sizeof(LFUNode<KeyType, ValueType>) + keyLength - 1;
    LFUNode<KeyType, ValueType>* newNode = reinterpret_cast<LFUNode<KeyType, ValueType>*>(malloc(nodeSize));
    newNode->deleter = deleter;
    newNode->key = new KeyType(key);
    newNode->value = value;
    newNode->charge = charge;
    newNode->keyLength = key.size();
    newNode->inCache = true;
    newNode->hash = hash;
    newNode->refs = 1;
    newNode->frequency = 1;
    std::memcpy(newNode->keyData, key.data(), keyLength);

    // Check if the key already exists in the cache
    auto it = lfuMap.find(key);
    if (it != lfuMap.end()) {
        // Key already exists, update the value and frequency
        LFUNode<KeyType, ValueType>* node = *(it->second);
        newNode->refs++;   // Remove the node from the frequency list
        // Add the new node to the frequency list
        lfuAppend(frequencyList[newNode->frequency], newNode);
        // Update the node in lfumap
        lfuMap[key] = frequencyList[newNode->frequency].begin();
        finishErase(node);
        usage_ += charge;
    }
    else {
        newNode->refs++;
        // Add the new node to the frequency list
        lfuAppend(frequencyList[newNode->frequency], newNode);
        // Update the node in lfumap
        lfuMap[key] = frequencyList[newNode->frequency].begin();
        usage_ += charge;
    }
    // erase the node from low frequency list to high frequency list until the usage is less than capacity
    while (usage_ > capacity_) {
        auto it = frequencyList.begin();
        if (it == it.end()) {
            // The frequency list is empty then delete the frequency list
            frequencyList.erase(it);
            continue;
        }
        LFUNode<KeyType, ValueType>* node = *(it->second).front();
        //lfuRemove(*(it->second), node);
        lfuMap.erase(*node->key);
        finishErase(node);
    }
    return reinterpret_cast<Handle*>(node);
}

template<typename KeyType, typename ValueType, typename LockType>
Handle* LFUCache<KeyType, ValueType, LockType>::lookUp(const KeyType& key, uint32_t hash) {
    std::lock_guard<LockType> lock(locker);

    // Check if the key exists in the cache
    auto it = lfuMap.find(key);
    if (it != lfuMap.end()) {
        LFUNode<KeyType, ValueType>* node = *(it->second);
        if (node->hash == hash) {
            // Update the frequency of the node
            ref(node);
            return reinterpret_cast<Handle*>(node);
        }
    }
    return reinterpret_cast<Handle*>(nullptr);
}


template<typename KeyType, typename ValueType, typename LockType>
void LFUCache<KeyType, ValueType, LockType>::release(Handle* handle) {
    std::lock_guard<LockType> lock(locker);
    LFUNode<KeyType, ValueType>* node = reinterpret_cast<LFUNode<KeyType, ValueType>*>(handle);
    unref(node);
}

template<typename KeyType, typename ValueType, typename LockType>
void LFUCache<KeyType, ValueType, LockType>::erase(const KeyType& key, uint32_t hash) {
    std::lock_guard<LockType> lock(locker);

    // Check if the key exists in the cache
    auto it = lfuMap.find(key);
    if (it != lfuMap.end()) {
        LFUNode<KeyType, ValueType>* node = *(it->second);
        if (node->hash == hash) {
            lfuMap.erase(it);
            finishErase(node);
        }
    }
}

template<typename KeyType, typename ValueType, typename LockType>
void LFUCache<KeyType, ValueType, LockType>::prune() {
    while (usage_ > capacity_) {
        auto it = frequencyList.begin();
        if (it == it.end()) {
            // The frequency list is empty then delete the frequency list
            frequencyList.erase(it);
            continue;
        }
        LFUNode<KeyType, ValueType>* node = *(it->second).front();
        //lfuRemove(*(it->second), node);
        lfuMap.erase(*node->key);
        finishErase(node);
    }
}

template<typename KeyType, typename ValueType, typename LockType>
void LFUCache<KeyType, ValueType, LockType>::lfuRemove(std::list<LFUNode<KeyType, ValueType>*>& list, LFUNode<KeyType, ValueType>* node) {
    list.erase(node);
}

template<typename KeyType, typename ValueType, typename LockType>
void LFUCache<KeyType, ValueType, LockType>::lfuAppend(std::list<LFUNode<KeyType, ValueType>*>& list, LFUNode<KeyType, ValueType>* node) {
    list.push_front(node);
}

template<typename KeyType, typename ValueType, typename LockType>
void LFUCache<KeyType, ValueType, LockType>::ref(LFUNode<KeyType, ValueType>* node) {
    node->refs++;
    lruRemove(frequencyList[node->frequency], node);
    node->frequency++;
    lruAppend(frequencyList[node->frequency], node);
}

template<typename KeyType, typename ValueType, typename LockType>
void LFUCache<KeyType, ValueType, LockType>::unref(LFUNode<KeyType, ValueType>* node) {
    node->refs--;
    if (node->refs == 0) {
        (*node->deleter)(*(node->key), node->value);
        free(node);
    }
}

template<typename KeyType, typename ValueType, typename LockType>
bool LFUCache<KeyType, ValueType, LockType>::finishErase(LFUNode<KeyType, ValueType>* node) {
    if (node != nullptr) {
        // Remove the node from the frequency list and LFU map
        lfuRemove(frequencyList[node->frequency], node);
        // Update the cache usage
        node->inCache = false;
        usage_ -= node->charge;
        unref(node);
    }
    return node != nullptr;
}