#pragma once
#pragma pack(1)

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

#define EVT_TIMER                           0x80000000
#define EVT_RUNTIME                         0x40000000
#define EVT_NOTIFY_WAIT                     0x00000100
#define EVT_NOTIFY_SIGNAL                   0x00000200
#define EVT_SIGNAL_EXIT_BOOT_SERVICES       0x00000201
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE   0x60000202

typedef void*       EFI_HANDLE;
typedef wchar_t     CHAR16;
typedef uint8_t     BOOLEAN;
typedef uint64_t    EFI_PHYSICAL_ADDRESS;
typedef uint64_t    EFI_VIRTUAL_ADDRESS;
typedef void*       EFI_EVENT;



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


typedef enum : uint32_t
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


typedef struct {
    uint32_t                        m_type;           // EFI_MEMORY_TYPE, Field size is 32 bits
    uint32_t                        m_pad;
    EFI_PHYSICAL_ADDRESS            m_physicalStart;  // Field size is 64 bits
    EFI_VIRTUAL_ADDRESS             m_virtualStart;   // Field size is 64 bits
    uint64_t                        m_numberOfPages;  // Field size is 64 bits
    uint64_t                        m_attribute;      // Field size is 64 bits
} EFI_MEMORY_DESCRIPTOR;


typedef enum 
{
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType 
} EFI_ALLOCATE_TYPE;

typedef uint64_t EFI_STATUS;

#pragma pack()
