#ifndef LRUNODE_HPP
#define LRUNODE_HPP
#include <cstdint>
#include <cstddef>
namespace OrangeKV {
    struct LRUNode {
        LRUNode* nextHash;
        LRUNode* next;
        LRUNode* prev;
        size_t length;
        uint32_t hash;
        bool in_cache;
    };
}
#endif 