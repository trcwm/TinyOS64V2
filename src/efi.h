// see: https://uefi.org/sites/default/files/resources/UEFI_Spec_2_8_final.pdf
#pragma once
#include <stdint.h>

typedef void* EFI_HANDLE;
typedef wchar_t CHAR16;

typedef struct EFI_TABLE_HEADER
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

typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL; // predeclaration
typedef uint64_t EFI_STATUS;

typedef EFI_STATUS (*EFI_TEXT_RESET)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *self, uint8_t extendedVerificiation);
typedef EFI_STATUS (*EFI_TEXT_STRING)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *self, CHAR16 *str);
typedef EFI_STATUS (*EFI_TEXT_CLEAR_SCREEN)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *self);

// see: "12.4 Simple Text Output Protocol"
typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
{
    EFI_TEXT_RESET          m_reset;
    EFI_TEXT_STRING         m_outputString;
    void*                   m_testString;
    void*                   m_queryMode;
    void*                   m_setMode;
    void*                   m_setAttribute;
    EFI_TEXT_CLEAR_SCREEN   m_clearScreen;
};

// ================================================================================
//   System Table
// ================================================================================

typedef struct EFI_SYSTEM_TABLE
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
};
