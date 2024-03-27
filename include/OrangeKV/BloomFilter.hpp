#ifndef BLOOMFILTER_HPP
#define BLOOMFILTER_HPP

#include <vector>
#include <string>
#include <unistd.h>
#include <cstdint>
#include <cstddef>
#include "utility/hash.hpp"
#include "include/OrangeKV/FilterPolicy.hpp"
namespace OrangeKV {
    static uint32_t BloomHash(const std::string& key) {
        return MurmurHash3_x86_32(key, key.size(), 0);
    }
    class BloomFilter : public FilterPolicy {
    private:
        size_t bitsPerKey;
        size_t k; // number of hash functions
    public:
        explicit BloomFilter(int bitsPerKey) : bitsPerKey(bitsPerKey) {
            k = static_cast<size_t>(bitsPerKey * 0.69); // ln(2) = 0.69
            if (k < 1) {
                k = 1;
            }
            if (k > 30) {
                k = 30;
            }
        }
        const std::string Name() const override {
            return "OrangeKV.BloomFilter";
        }
        void createFilter(const std::string& keys, size_t n, std::string& dest) override {
            // n is the number of keys
            size_t bits = n * bitsPerKey;
            if (bits < 64) {
                bits = 64;
            }
            size_t bytes = (bits + 7) / 8;
            bits = bytes * 8;
            std::vector<bool> bitsVec(bits, false);
            for (size_t i = 0; i < n; i++) {
                uint32_t hash = BloomHash(std::string(1, keys[i]));// 
                uint32_t delta = (hash >> 17) | (hash << 15);
                for (size_t j = 0; j < k; j++) {
                    size_t bitPos = hash % bits;
                    bitsVec[bitPos] =  bitsVec[bitPos] || true;
                    hash += delta;
                }
            }
        }
        bool keyMayMatch(const std::string& key, const std::string& filter) override {
            size_t bits = filter.size() * 8;
            if (bits < 64) {
                return false;
            }
            uint32_t hash = BloomHash(key);
            uint32_t delta = (hash >> 17) | (hash << 15);
            for (size_t i = 0; i < k; i++) {
                size_t bitPos = hash % bits;
                if ((filter[bitPos / 8] & (1 << (bitPos % 8))) == 0) {
                    return false;
                }
                hash += delta;
            }
            return true;
        }
    };
}


#endif // BLOOMFILTER_HPP