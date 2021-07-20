#include <cstdint>
#include <uchar.h>
#include "efi.h"

extern "C"
{
    __attribute__((naked)) void efi_init(EFI_HANDLE handle, EFI_SYSTEM_TABLE *sysTbl)
    {
        sysTbl->m_conOut->m_clearScreen(sysTbl->m_conOut);

        const wchar_t*str = L"Hello, world!\n";
        sysTbl->m_conOut->m_outputString(sysTbl->m_conOut, (CHAR16*)str);
        
        while(1) {}
    }
}
