#ifndef HASH_HPP
#define HASH_HPP
#include <unistd.h>
#include <cstdint>
#include <cstddef>
#include <string> 
namespace OrangeKV {
    uint32_t HashBKDR(const std::string& data, size_t n, uint32_t seed);
    uint32_t HashDJB(const std::string& data, size_t n, uint32_t seed);
    uint32_t HashSDBM(const std::string& data, size_t n, uint32_t seed);
    uint32_t HashAP(const std::string& data, size_t n, uint32_t seed);
    uint32_t MurmurHash3_x86_32(const std::string& data, size_t n, uint32_t seed);
    uint32_t MurmurHash3_x86_64(const std::string& data, size_t n, uint32_t seed); 
    /**
     * @brief Calculates the BKDR hash value for the given data.
     * 
     * @param data The input string.
     * @param n The length of the input string.
     * @param seed The initial seed value for the hash calculation.
     * @return The calculated BKDR hash value.
     */
    uint32_t HashBKDR(const std::string& data, size_t n, uint32_t seed = 131) {
        uint32_t hash = 0;
        for (size_t i = 0; i < n; i++) {
            hash = (hash * seed) + data[i];
        }
        return hash;
    }
    

    /**
     * @brief Calculates the DJB hash value for the given data.
     * 
     * @param data The input string.
     * @param n The length of the input string.
     * @param seed The initial seed value for the hash calculation.
     * @return The calculated DJB hash value.
     */
    uint32_t HashDJB(const std::string& data, size_t n, uint32_t seed = 5381) {
        uint32_t hash = seed;
        for (size_t i = 0; i < n; i++) {
            hash = ((hash << 5) + hash) + data[i];
        }
        return hash;
    }


    /**
     * @brief Calculates the SDBM hash value for the given data.
     * 
     * @param data The input string.
     * @param n The length of the input string.
     * @param seed The initial seed value for the hash calculation.
     * @return The calculated SDBM hash value.
     */
    uint32_t HashSDBM(const std::string& data, size_t n, uint32_t seed = 0) {
        uint32_t hash = seed;
        for (size_t i = 0; i < n; i++) {
            hash = data[i] + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }


    /**
     * @brief Calculates the AP hash value for the given data.
     * 
     * @param data The input string.
     * @param n The length of the input string.
     * @param seed The initial seed value for the hash calculation.
     * @return The calculated AP hash value.
     */
    uint32_t HashAP(const std::string& data, size_t n, uint32_t seed = 0) {
        uint32_t hash = seed;
        for (size_t i = 0; i < n; i++) {
            if (i & 1) {
                hash ^= ((hash << 7) ^ data[i] ^ (hash >> 3));
            } else {
                hash ^= (~((hash << 11) ^ data[i] ^ (hash >> 5)));
            }
        }
        return hash;
    }


    /**
     * @brief Calculates the MurmurHash3_x86_32 hash value for the given data.
     * 
     * @param data The input string.
     * @param n The length of the input string.
     * @param seed The initial seed value for the hash calculation.
     * @return The calculated MurmurHash3_x86_32 hash value.
     */
    uint32_t MurmurHash3_x86_32(const std::string& data, size_t n, uint32_t seed = 0) {
        const uint32_t c1 = 0xcc9e2d51;
        const uint32_t c2 = 0x1b873593;
        const uint32_t r1 = 15;
        const uint32_t r2 = 13;
        const uint32_t m = 5;
        const uint32_t n1 = 0xe6546b64;

        uint32_t hash = seed;
        size_t len = n;

        const uint32_t* chunks = reinterpret_cast<const uint32_t*>(data.c_str());
        const size_t numChunks = len / 4;

        for (size_t i = 0; i < numChunks; ++i) {
            uint32_t k = chunks[i];
            k *= c1;
            k = (k << r1) | (k >> (32 - r1));
            k *= c2;

            hash ^= k;
            hash = ((hash << r2) | (hash >> (32 - r2))) * m + n1;
        }

        const uint8_t* tail = reinterpret_cast<const uint8_t*>(data.c_str() + numChunks * 4);
        uint32_t k1 = 0;
        //Case 3, 2, 1 all will be executed!
        //The attribute [[fallthrough]] in C++17
        switch (len & 3) {
            case 3:
                k1 ^= tail[2] << 16;
                [[fallthrough]];
            case 2:
                k1 ^= tail[1] << 8;
                [[fallthrough]];
            case 1:
                k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << r1) | (k1 >> (32 - r1));
                k1 *= c2;
                hash ^= k1;
        }
        hash ^= len;
        hash ^= (hash >> 16);
        hash *= 0x85ebca6b;
        hash ^= (hash >> 13);
        hash *= 0xc2b2ae35;
        hash ^= (hash >> 16);
        return hash;
    }


    /**
     * @brief Calculates the MurmurHash3_x86_64 hash value for the given data.
     * 
     * @param data The input string.
     * @param n The length of the input string.
     * @param seed The initial seed value for the hash calculation.
     * @return The calculated MurmurHash3_x86_64 hash value.
     */
    uint32_t MurmurHash3_x86_64(const std::string& data, size_t n, uint32_t seed = 0) {
        const uint64_t c1 = 0x87c37b91114253d5;
        const uint64_t c2 = 0x4cf5ad432745937f;
        const uint32_t r1 = 31;
        const uint32_t r2 = 27;
        const uint32_t r3 = 33;
        const uint32_t m = 5;
        const uint32_t n1 = 0x52dce729;
        const uint32_t n2 = 0x38495ab5;

        uint64_t hash = seed;
        size_t len = n;

        const uint64_t* chunks = reinterpret_cast<const uint64_t*>(data.c_str());
        const size_t numChunks = len / 8;

        for (size_t i = 0; i < numChunks; ++i) {
            uint64_t k = chunks[i];
            k *= c1;
            k = (k << r1) | (k >> (64 - r1));
            k *= c2;

            hash ^= k;
            hash = ((hash << r2) | (hash >> (64 - r2))) * m + n1;
        }

        const uint8_t* tail = reinterpret_cast<const uint8_t*>(data.c_str() + numChunks * 8);
        uint64_t k1 = 0;

        switch (len & 7) {
            case 7:
                k1 ^= static_cast<uint64_t>(tail[6]) << 48;
                [[fallthrough]];
            case 6:
                k1 ^= static_cast<uint64_t>(tail[5]) << 40;
                [[fallthrough]];
            case 5:
                k1 ^= static_cast<uint64_t>(tail[4]) << 32;
                [[fallthrough]];
            case 4:
                k1 ^= static_cast<uint64_t>(tail[3]) << 24;
                [[fallthrough]];
            case 3:
                k1 ^= static_cast<uint64_t>(tail[2]) << 16;
                [[fallthrough]];
            case 2:
                k1 ^= static_cast<uint64_t>(tail[1]) << 8;
                [[fallthrough]];
            case 1:
                k1 ^= static_cast<uint64_t>(tail[0]);
                k1 *= c1;
                k1 = (k1 << r1) | (k1 >> (64 - r1));
                k1 *= c2;
                hash ^= k1;
        }

        hash ^= len;
        hash ^= (hash >> r3);
        hash *= n2;
        hash ^= (hash >> r2);
        hash *= n1;
        hash ^= (hash >> r3);
        return static_cast<uint32_t>(hash);
    }
} // End of namespace
#endif // HASH_HPP