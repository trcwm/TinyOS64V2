#pragma once

#include "efi_base.h"

// ================================================================================
//   Boot services
// ================================================================================

struct EFI_BOOT_SERVICES;    // pre declaration

typedef EFI_STATUS (*EFI_LOCATE_PROTOCOL)(UEFI_GUID *Protocol, void *registration /* optional */, void **interface);

typedef EFI_STATUS (*EFI_GET_MEMORY_MAP)(
    uint64_t                *m_memoryMapSize,
    EFI_MEMORY_DESCRIPTOR   *m_memoryMap,
    uint64_t                *m_mapKey,
    uint64_t                *m_descriptorSize,   
    uint32_t                *m_descriptorVersion
);

typedef EFI_STATUS(*EFI_ALLOCATE_POOL)(   
    EFI_MEMORY_TYPE     m_poolType,
    uint64_t            m_size,
    void                **m_buffer
);

typedef EFI_STATUS(*EFI_FREE_POOL)(   
    void                *m_buffer
);

typedef void(*EFI_EVENT_NOTIFY)(   
    EFI_EVENT m_event,
    void     *m_context
);

typedef EFI_STATUS (*EFI_HANDLE_PROTOCOL)(EFI_HANDLE handle, UEFI_GUID *protocol, void **interface);

typedef EFI_STATUS (*EFI_WAIT_FOR_EVENT)(
    uint64_t    numberOfEvents, 
    EFI_EVENT   *event,
    uint64_t    *index  /* out */
);

typedef EFI_STATUS (*EFI_SET_WATCHDOG_TIMER)(
    uint64_t    timeout,
    uint64_t    watchdogCode,
    uint64_t    dataSize,
    CHAR16*     watchdogData
);

typedef EFI_STATUS (*EFI_ALLOCATE_PAGES)(
    EFI_ALLOCATE_TYPE Type,
    EFI_MEMORY_TYPE MemoryType,
    uint64_t Pages,
    EFI_PHYSICAL_ADDRESS* Memory
);

typedef EFI_STATUS (*EFI_FREE_PAGES)(
    uint64_t Pages,
    EFI_PHYSICAL_ADDRESS Memory
);

struct EFI_BOOT_SERVICES
{
    EFI_TABLE_HEADER        m_header;
    void*                   m_raiseTpl;
    void*                   m_restoreTpl;

    // memory services
    EFI_ALLOCATE_PAGES      m_allocatePages;
    EFI_FREE_PAGES          m_freePages;
    EFI_GET_MEMORY_MAP      m_getMemoryMap;
    EFI_ALLOCATE_POOL       m_allocatePool;
    EFI_FREE_POOL           m_freePool;

    // event and timer services
    void*                   m_createEvent;
    void*                   m_setTimer;
    EFI_WAIT_FOR_EVENT      m_waitforEvent;
    void*                   m_signalEvent;
    void*                   m_closeEvent;
    void*                   m_checkEvent;

    // protocol handler servies
    void*                   m_installProtocolInterface;
    void*                   m_reinstallProtocolInterface;
    void*                   m_uninstallProtocolInterface;
    EFI_HANDLE_PROTOCOL     m_handleProtocol;
    void*                   m_reserved;
    void*                   m_registerProtocolNotify;
    void*                   m_locateHandle;
    void*                   m_locateDevicePath;
    void*                   m_installConfigurationTable;

    // image services
    void*                   m_loadImage;
    void*                   m_startImage;
    void*                   m_exit;
    void*                   m_unloadImage;
    void*                   m_exitBootServices;

    // misc services
    void*                   m_getNextMonotonicCount;
    void*                   m_stall;
    EFI_SET_WATCHDOG_TIMER  m_setWatchdogTimer;

    // driver support
    void*                   m_connectController;
    void*                   m_disconnectController;

    // open/close protocol serives
    void*                   m_openProtocol;
    void*                   m_closeProtocol;
    void*                   m_openProtocolInformation;

    // Library services
    void*                   m_protocolsPerHandle;
    void*                   m_locateHandleBuffer;
    EFI_LOCATE_PROTOCOL     m_locateProtocol;
};




struct EFI_LOADED_IMAGE_PROTOCOL
{ 
    uint32_t                  Revision;
    EFI_HANDLE                ParentHandle;
    void                     *SystemTable;
    
    // Source location of the image 
    EFI_HANDLE                DeviceHandle;
    //EFI_DEVICE_PATH_PROTOCOL  *FilePath;
    void                      *FilePath;
    void                      *Reserved;
    
    // Image’s load options 
    uint32_t                   LoadOptionsSize;
    void                      *LoadOptions;
    // Location where image was loaded 
    void                      *ImageBase;
    uint64_t                  ImageSize;
    EFI_MEMORY_TYPE           ImageCodeType;
    EFI_MEMORY_TYPE           ImageDataType;
    //EFI_IMAGE_UNLOAD          Unload;
    void*                     Unload;
};