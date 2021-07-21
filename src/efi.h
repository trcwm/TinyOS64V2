#pragma once

#include "efi_base.h"
#include "efi_console.h"
#include "efi_bootservices.h"

#define EFI_ACPI_20_TABLE_GUID {0x8868e871,0xe4f1,0x11d3, {0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}}
#define ACPI_TABLE_GUID        {0xeb9d2d30,0x2d88,0x11d3, {0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
#define SAL_SYSTEM_TABLE_GUID  {0xeb9d2d32,0x2d88,0x11d3, {0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
#define SMBIOS_TABLE_GUID      {0xeb9d2d31,0x2d88,0x11d3, {0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
#define SMBIOS3_TABLE_GUID     {0xf2fd1544,0x9794,0x4a2c, {0x99,0x2e,0xe5,0xbb,0xcf,0x20,0xe3,0x94}}
#define MPS_TABLE_GUID         {0xeb9d2d2f,0x2d88,0x11d3, {0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}

//// ACPI 2.0 or newer tables should use EFI_ACPI_TABLE_GUID
//#define EFI_ACPI_TABLE_GUID    {0x8868e871,0xe4f1,0x11d3, {0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}}
//#define EFI_ACPI_20_TABLE_GUID EFI_ACPI_TABLE_GUID
#define ACPI_TABLE_GUID        {0xeb9d2d30,0x2d88,0x11d3, {0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
#define ACPI_10_TABLE_GUID ACPI_TABLE_GUID

#define EFI_LOADED_IMAGE_PROTOCOL_GUID {0x5B1B31A1,0x9562,0x11d2, {0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B}}

// ================================================================================
//   System Table
// ================================================================================

struct EFI_CONFIGURATION_TABLE
{ 
    UEFI_GUID           m_vendorGuid;
    void               *m_vendorTable;
};

struct EFI_SYSTEM_TABLE
{
    EFI_TABLE_HEADER  m_header;
    CHAR16           *m_firmwareVendor;
    uint32_t          m_firmwareRevision;
    EFI_HANDLE        m_consoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL*  m_conIn;
    EFI_HANDLE        m_consoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* m_conOut;
    EFI_HANDLE        m_standardErrorHandle;
    void*             m_stdErr;
    void*             m_runtimeServices;
    EFI_BOOT_SERVICES* m_bootServices;
    uint64_t          m_numberOfTableEntries;
    EFI_CONFIGURATION_TABLE *m_configurationTable;
};
