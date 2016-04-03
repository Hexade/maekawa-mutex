#ifndef SAFE_VECTOR_TPP
#define SAFE_VECTOR_TPP

#include "utils.h"

template <typename T>
void SafeVector<T>::push_back(T value)
{
    std::lock_guard<std::mutex> lock(m);
    v.push_back(value);
}

template <typename T>
void SafeVector<T>::remove(T value)
{
    std::lock_guard<std::mutex> lock(m);
    for (typename std::vector<T>::iterator it = v.begin(); it != v.end();) {
        if (*it == value) {
            it = v.erase(it);
        } else
            ++it;
    }
}

template <typename T>
unsigned int SafeVector<T>::size(void)
{
    std::lock_guard<std::mutex> lock(m);
    return v.size();
}

template <typename T>
bool SafeVector<T>::contains(T value)
{
    std::lock_guard<std::mutex> lock(m);
    for (const auto& i: v) {
        if (i == value)
            return true;
    }
    return false;
}

template <typename T>
void SafeVector<T>::clear(void)
{
    std::lock_guard<std::mutex> lock(m);
    v.clear();
}

template <typename T>
void SafeVector<T>::print(const std::string& name, int from)
{
    std::lock_guard<std::mutex> lock(m);
    Utils::print_vector(name, from, v);
}

#endif
