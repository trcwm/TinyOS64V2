// see: https://uefi.org/sites/default/files/resources/UEFI_Spec_2_8_final.pdf
#pragma once

#include "efi_base.h"
#include <stdlib.h>

// ================================================================================
//   Text Output Procotol
// ================================================================================

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL; // predeclaration

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

struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL
{ 
    uint8_t  Blue;
    uint8_t  Green;
    uint8_t  Red;
    uint8_t  Reserved;
};

typedef enum 
{ 
    EfiBltVideoFill, 
    EfiBltVideoToBltBuffer, 
    EfiBltBufferToVideo,  
    EfiBltVideoToVideo, 
    EfiGraphicsOutputBltOperationMax
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

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
typedef EFI_STATUS (*EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE)(EFI_GRAPHICS_OUTPUT_PROTOCOL *self, uint32_t modeNumber);

typedef EFI_STATUS (*EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT)(
    EFI_GRAPHICS_OUTPUT_PROTOCOL    *self, 
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *BltBuffer /* optional */,
    EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
    uint64_t sourceX, uint64_t sourceY,
    uint64_t destX, uint64_t destY,
    uint64_t width, uint64_t height,
    uint64_t delta /* optional */
);

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
    EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE   SetMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT        Blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE       *Mode;
};
