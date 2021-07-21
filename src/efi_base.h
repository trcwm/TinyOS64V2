#pragma once

#include <cstdint>

typedef void*       EFI_HANDLE;
typedef wchar_t     CHAR16;
typedef uint8_t     BOOLEAN;
typedef uint64_t    EFI_PHYSICAL_ADDRESS;

struct UEFI_GUID {
   uint32_t  Data1;
   uint16_t  Data2;
   uint16_t  Data3;
   uint8_t   Data4[8];
};

struct EFI_TABLE_HEADER
{
    uint64_t m_signature;       ///< unique ID
    uint32_t m_revision;
    uint32_t m_headerSize;      ///< in bytes, includes everything
    uint32_t m_crc32;
    uint32_t m_reserved;        ///< must be zero    
};

typedef enum 
{  
    EfiReservedMemoryType,  
    EfiLoaderCode,  
    EfiLoaderData,  
    EfiBootServicesCode,  
    EfiBootServicesData,  
    EfiRuntimeServicesCode,  
    EfiRuntimeServicesData,  
    EfiConventionalMemory,  
    EfiUnusableMemory,  
    EfiACPIReclaimMemory,  
    EfiACPIMemoryNVS,  
    EfiMemoryMappedIO,  
    EfiMemoryMappedIOPortSpace,  
    EfiPalCode,  
    EfiPersistentMemory,  
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;


typedef uint64_t EFI_STATUS;

