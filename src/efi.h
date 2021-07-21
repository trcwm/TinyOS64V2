#pragma once

#include "efi_base.h"
#include "efi_console.h"
#include "efi_bootservices.h"

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
