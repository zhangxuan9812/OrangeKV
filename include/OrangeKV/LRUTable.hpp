#ifndef LRUTABLE_HPP
#define LRUTABLE_HPP
#include <cstdint>
#include <cstddef>
#include "LRUNode.hpp"
namespace OrangeKV {
    class Table {
    private:
        uint32_t buckets;
        uint32_t elements;
        LRUNode** arr;
    public:
        Table() {
            buckets = 0;
            elements = 0;
            arr = nullptr;
        }
        ~Table() {
        }
    private:
        /**
         * Resizes the LRUTable by doubling the number of buckets and rehashing the elements.
         * This function is called when the number of elements in the table exceeds the load factor threshold.
         */
        void resize() {
            uint32_t newBuckets = buckets * 2;
            LRUNode** newArr = new LRUNode*[newBuckets]; // as array of double-linked lists
            for (uint32_t i = 0; i < newBuckets; i++) {
                newArr[i] = nullptr;
            }
            
            for (uint32_t i = 0; i < buckets; i++) {
                LRUNode* curr = arr[i];
                while (curr != nullptr) {
                    LRUNode* next = curr->next;
                    uint32_t newIndex = curr->hash % newBuckets;
                    curr->next = newArr[newIndex];
                    newArr[newIndex] = curr;
                    curr = next;
                }
            }
            
            delete[] arr;
            arr = newArr;
            buckets = newBuckets;
        }
    };
} 

#endif 