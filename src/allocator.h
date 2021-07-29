#pragma once
#include <cstdint>
#include <cstdlib>

void* operator new (std::size_t size);
void operator delete(void*);

//extern "C"
//{
//    void memmove(void *s1, const void *s2, size_t n);
//}

namespace std
{
    __attribute (( noreturn)) void __throw_bad_alloc();
    __attribute (( noreturn)) void __throw_length_error(char const *);
};

