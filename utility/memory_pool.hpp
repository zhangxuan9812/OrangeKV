#ifndef MEMORY_POOL_HPP
#define MEMORY_POOL_HPP

// Include any necessary headers here
#include <iostream>
#include <vector>
#include <memory>
namespace OrangeKV {
    class MemoryPool {
    public:
        MemoryPool();
        MemoryPool(const MemoryPool&) = delete;
        MemoryPool& operator=(const MemoryPool&) = delete;
        ~MemoryPool();
    private:
        char* allocate(size_t bytes)
    };
}
// Define your memory pool class here

#endif // MEMORY_POOL_HPP