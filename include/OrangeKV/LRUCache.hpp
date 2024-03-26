#ifndef ORANGEKV_LRUCACHE_HPP
#define ORANGEKV_LRUCACHE_HPP
#include <unordered_map>
#include <list>

namespace OrangeKV {

    template<typename KeyType, typename ValueType, typename LockType>
    class LRUCache {
    public:
        LRUCache();
        ~LRUCache();

        void put(const KeyType& key, const ValueType& value);
        ValueType get(const KeyType& key);

    private:
        const size_t capacity = 0;
        mutable LockType locker;
        std::unordered_map<KeyType, std::pair<ValueType, typename std::list<KeyType>::iterator>> cache_;
        std::list<KeyType> lruList_;

        void evict();
    };

    template<typename KeyType, typename ValueType, typename LockType>
    LRUCache<KeyType, ValueType, LockType>::LRUCache() {
        capacity = 0;
    }

} 
#endif