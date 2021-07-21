#pragma once

#include "efi_base.h"

// ================================================================================
//   Boot services
// ================================================================================

struct EFI_BOOT_SERVICES;    // pre declaration

typedef EFI_STATUS (*EFI_LOCATE_PROTOCOL)(UEFI_GUID *Protocol, void *registration /* optional */, void **interface);

struct EFI_BOOT_SERVICES
{
    EFI_TABLE_HEADER        m_header;
    void*                   m_raiseTpl;
    void*                   m_restoreTpl;

    // memory services
    void*                   m_allocatePages;
    void*                   m_freePages;
    void*                   m_getMemoryMap;
    void*                   m_allocatePool;
    void*                   m_freePool;

    // event and timer services
    void*                   m_createEvent;
    void*                   m_setTimer;
    void*                   m_waitforEvent;
    void*                   m_signalEvent;
    void*                   m_closeEvent;
    void*                   m_checkEvent;

    // protocol handler servies
    void*                   m_installProtocolInterface;
    void*                   m_reinstallProtocolInterface;
    void*                   m_uninstallProtocolInterface;
    void*                   m_handleProtocol;
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
    void*                   m_setWatchdogTimer;

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

