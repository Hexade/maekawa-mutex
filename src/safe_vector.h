#ifndef SAFE_VECTOR_H
#define SAFE_VECTOR_H

#include <mutex>
#include <vector>
#include <string>

/*
 * Thread safe implementation of std::vector
*/
template <typename T>
class SafeVector
{
    public:
        void push_back(T value);
        void remove(T value);
        unsigned int size(void);
        bool contains(T value);
        void clear(void);
        void print(const std::string& name, int from);

    private:
        std::vector<T> v;
        std::mutex m;        
};

// SafeVector template implementation
#include "safe_vector.tpp"

#endif
