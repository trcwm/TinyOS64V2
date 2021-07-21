#pragma once

#include <cstdint>

#define BITS_PER_LONG 64

#define EFI_SUCCESS		0
#define EFI_LOAD_ERROR          ( 1 | (1UL << (c-1)))
#define EFI_INVALID_PARAMETER	( 2 | (1UL << (BITS_PER_LONG-1)))
#define EFI_UNSUPPORTED		    ( 3 | (1UL << (BITS_PER_LONG-1)))
#define EFI_BAD_BUFFER_SIZE     ( 4 | (1UL << (BITS_PER_LONG-1)))
#define EFI_BUFFER_TOO_SMALL	( 5 | (1UL << (BITS_PER_LONG-1)))
#define EFI_NOT_READY		    ( 6 | (1UL << (BITS_PER_LONG-1)))
#define EFI_DEVICE_ERROR	    ( 7 | (1UL << (BITS_PER_LONG-1)))
#define EFI_WRITE_PROTECTED	    ( 8 | (1UL << (BITS_PER_LONG-1)))
#define EFI_OUT_OF_RESOURCES	( 9 | (1UL << (BITS_PER_LONG-1)))
#define EFI_NOT_FOUND		    (14 | (1UL << (BITS_PER_LONG-1)))
#define EFI_TIME_OUT		    (18 | (1UL << (BITS_PER_LONG-1)))
#define EFI_ABORTED		        (21 | (1UL << (BITS_PER_LONG-1)))
#define EFI_PROTOCOL_ERROR      (24 | (1UL << (BITS_PER_LONG-1)))
#define EFI_SECURITY_VIOLATION	(26 | (1UL << (BITS_PER_LONG-1)))


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

typedef enum 
{
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType 
} EFI_ALLOCATE_TYPE;

typedef uint64_t EFI_STATUS;

