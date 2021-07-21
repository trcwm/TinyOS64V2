// see: https://uefi.org/sites/default/files/resources/UEFI_Spec_2_8_final.pdf
#pragma once
#include <stdint.h>
#include <stdlib.h>

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

// ================================================================================
//   Text Output Procotol
// ================================================================================

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL; // predeclaration
typedef uint64_t EFI_STATUS;

typedef EFI_STATUS (*EFI_TEXT_RESET)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *self, BOOLEAN extendedVerificiation);
typedef EFI_STATUS (*EFI_TEXT_STRING)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *self, CHAR16 *str);
typedef EFI_STATUS (*EFI_TEXT_CLEAR_SCREEN)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *self);
typedef EFI_STATUS (*EFI_TEXT_ENABLE_CURSOR)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *self, BOOLEAN visible);

// see: "12.4 Simple Text Output Protocol"
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
{
    EFI_TEXT_RESET          m_reset;
    EFI_TEXT_STRING         m_outputString;
    void*                   m_testString;
    void*                   m_queryMode;
    void*                   m_setMode;
    void*                   m_setAttribute;
    EFI_TEXT_CLEAR_SCREEN   m_clearScreen;
    void*                   m_setCursorPosition;
    EFI_TEXT_ENABLE_CURSOR  m_enableCursor;
};

// ================================================================================
//   Graphics Output Procotol
// ================================================================================

struct EFI_GRAPHICS_OUTPUT_PROTOCOL;    // pre-declaration

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID {0x9042a9de,0x23dc,0x4a38,  {0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}}

struct EFI_PIXEL_BITMASK
{ 
    uint32_t   RedMask;
    uint32_t   GreenMask;
    uint32_t   BlueMask;
    uint32_t   ReservedMask;
};

typedef enum
{ 
    PixelRedGreenBlueReserved8BitPerColor, 
    PixelBlueGreenRedReserved8BitPerColor, 
    PixelBitMask, 
    PixelBltOnly, 
    PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION
{
    uint32_t m_version;
    uint32_t m_horizontalResolution;
    uint32_t m_verticalResolution;
    EFI_GRAPHICS_PIXEL_FORMAT m_pixelFormat;
    EFI_PIXEL_BITMASK m_pixelInformation;
    uint32_t m_pixelsPerScanline;
};

typedef EFI_STATUS (*EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE)(EFI_GRAPHICS_OUTPUT_PROTOCOL *self, uint32_t modeNumber, size_t *sizeOfInfo, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **info);
//typedef EFI_STATUS (*EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE)(UEFI_GUID *Protocol, void *registration /* optional */, void **interface);
//typedef EFI_STATUS (*EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT)(UEFI_GUID *Protocol, void *registration /* optional */, void **interface);
//typedef EFI_STATUS (*EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE)(UEFI_GUID *Protocol, void *registration /* optional */, void **interface);

struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE
{
    uint32_t    m_maxMode;
    uint32_t    m_mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION     *m_info;
    uint64_t    m_sizeOfInfo;
    EFI_PHYSICAL_ADDRESS    m_frameBufferBase;
    uint64_t    m_frameBufferSize;
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE QueryMode; 
    void* m_setMode;
    void* m_blt;
    //EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE   SetMode;   
    //EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT        Blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE       *Mode;
};

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

// ================================================================================
//   System Table
// ================================================================================

struct EFI_SYSTEM_TABLE
{
    EFI_TABLE_HEADER  m_header;
    CHAR16           *m_firmwareVendor;
    uint32_t          m_firmwareRevision;
    EFI_HANDLE        m_consoleInHandle;
    void*             m_conIn;
    EFI_HANDLE        m_consoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* m_conOut;
    EFI_HANDLE        m_standardErrorHandle;
    void*             m_stdErr;
    void*             m_runtimeServices;
    EFI_BOOT_SERVICES* m_bootServices;
};
