#ifndef ORANGEKV_FILTERPOLICY_HPP
#define ORANGEKV_FILTERPOLICY_HPP
#include <cstdint>
#include <cstddef>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>   
#include "utility/hash.hpp"
class FilterPolicy {
public:
    FilterPolicy() {}
    virtual ~FilterPolicy();
    virtual const std::string Name() const = 0;
    virtual void createFilter(const std::string& keys, size_t n, std::string& dest) = 0;
    virtual bool keyMayMatch(const std::string& key, const std::string& filter) = 0;
};
const FilterPolicy* NewBloomFilterPolicy(int bits_per_key);
#endif 