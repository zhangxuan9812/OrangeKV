#include <iostream>
using namespace std;
int main()
{
    cout << "Hello, World!" << endl;
    return 0;
}
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
    [[nodiscard]] std::string getkey() const {
        return std::string(keyData, keyLength);
    }
};

struct Handle{};


template<typename KeyType, typename ValueType, typename LockType>
class LRUCache {
private:
    size_t capacity_;
    size_t usage_;
    std::list<LRUNode<KeyType, ValueType>> lruList;
    std::list<LRUNode<KeyType, ValueType>> inUseList;
    std::unordered_map<KeyType, typename std::list<LRUNode<KeyType, ValueType>>::iterator, OrangeKV::HashBKDR> lruMap;
    LockType locker;
public:
    LRUCache();
    ~LRUCache();
    Handle* insert(const KeyType& key, uint32_t hash, ValueType* value, size_t charge, void (*deleter)(const KeyType& key, ValueType* value));
    Handle* lookUp(const KeyType& key, uint32_t hash);
    void release(Handle* handle);
    void erase(const KeyType& key, uint32_t hash);
    void prune();
    size_t capacity() const {
        return capacity_;
    }
    size_t totalCharge() const {
        return usage_;
    }
private:
    void lruRemove(LRUNode<KeyType, ValueType>* node);
    void lruAppend(std::list<LRUNode<KeyType, ValueType>*>& list, LRUNode<KeyType, ValueType>* node);
    void ref(LRUNode<KeyType, ValueType>* node);
    void unref(LRUNode<KeyType, ValueType>* node);
    bool finshErase(LRUNode<KeyType, ValueType>* node);
};


 //ORANGEKV_LRU_HPP



 #include <cstdint>
#include <cstddef>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <memory> 
#include "utility/hash.hpp"


//The type of node
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