#pragma once
#include <cstdint>

namespace ACPI
{

#pragma pack(1)

struct RSDP
{
    const char  m_signature[8];     // must be "RSD PTR ";
    uint8_t     m_checksum;
    const char  m_oemID[6];         // OEM string
    uint8_t     m_revision;
    uint32_t    m_rsdtAddress;
    uint32_t    m_length;
    uint64_t    m_xsdtAddress;
    uint8_t     m_extendedChecksum;
    uint8_t     m_reserved[3];
};

struct ISDTHeader {
  const char    m_signature[4];
  uint32_t      m_length;           // size of the table incl. the header
  uint8_t       m_revision;
  uint8_t       m_checksum;
  const char    m_oemID[6];
  const char    m_oemTableID[8];
  uint32_t      m_oemRevision;
  uint32_t      m_creatorID;
  uint32_t      m_creatorRevision;
};

struct RSDT 
{
    ISDTHeader  m_header;
    uint32_t    m_sdtPointers[];
};

struct XSDT 
{
    ISDTHeader  m_header;
    uint64_t    m_sdtPointers[];
};

#pragma pack()

};