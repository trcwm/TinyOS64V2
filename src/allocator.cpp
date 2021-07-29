#include "allocator.h"
#include "efi.h"

extern EFI_SYSTEM_TABLE *g_sysTbl;
extern size_t print(const char *fmt, ...);

void* operator new (std::size_t size) 
{
    void* bufferPtr = 0;
    auto status = g_sysTbl->m_bootServices->m_allocatePool(EfiRuntimeServicesData, size, (void**)&bufferPtr);
    if (status == EFI_SUCCESS)
    {
        //print("ALLOC %d bytes\n\r", size);
        return bufferPtr;
    }

    std::__throw_bad_alloc();
}

void operator delete(void* buffer)
{
    auto status = g_sysTbl->m_bootServices->m_freePool(buffer);
}

void std::__throw_bad_alloc() 
{ 
    print("!! BAD ALLOC!\n\r");
    while(1) {}; 
};

void std::__throw_length_error(char const *)
{
    while(1) {}; 
};

extern "C"
{
    void memmove(void *s1, const void *s2, size_t n)
    {
        uint8_t* dst = (uint8_t*)s1;
        const uint8_t* src = (const uint8_t*)s2;
        while(n > 0)
        {
            *dst++ = *src++;
            n--;
        }
    }

#if 1
    int memcmp(const char *s1, const char *s2, size_t count)
    {
        while (count-- > 0)
        {
            if (*s1++ != *s2++)
                return s1[-1] < s2[-1] ? -1 : 1;
        }
        return 0;    
    }
#endif
}
