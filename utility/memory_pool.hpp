#ifndef MEMORY_POOL_HPP
#define MEMORY_POOL_HPP

// Include any necessary headers here
#include <iostream>
#include <vector>
#include <memory>
namespace OrangeKV
{
    class MemoryPool
    {
    public:
        MemoryPool(size_t size) : size_(size)
        {
            pool_ = std::make_unique<char[]>(size_);
        }
        ~MemoryPool() = default;
        void *Allocate(size_t size)
        {
            if (size > size_)
            {
                return nullptr;
            }
            if (size_ - offset_ < size)
            {
                return nullptr;
            }
            void *ptr = pool_.get() + offset_;
            offset_ += size;
            return ptr;
        }
        void Free(void *ptr)
        {
            // Do nothing
        }
    }
}
// Define your memory pool class here

#endif // MEMORY_POOL_HPP