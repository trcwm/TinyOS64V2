#pragma once

#include <cstdint>
#include "efi_base.h"

// ========================================================================
//
//   EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL
//
// ========================================================================

#define EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GUID \ {0x2F707EBB,0x4A1A,0x11d4,\  {0x9A,0x38,0x00,0x90,0x27,0x3F,0xC1,0x4D}}

struct EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL; // pre-declaration

typedef enum 
{ 
    EfiPciWidthUint8, 
    EfiPciWidthUint16, 
    EfiPciWidthUint32, 
    EfiPciWidthUint64, 
    EfiPciWidthFifoUint8, 
    EfiPciWidthFifoUint16, 
    EfiPciWidthFifoUint32, 
    EfiPciWidthFifoUint64,
    EfiPciWidthFillUint8, 
    EfiPciWidthFillUint16, 
    EfiPciWidthFillUint32, 
    EfiPciWidthFillUint64, 
    EfiPciWidthMaximum
} EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH;

typedef EFI_STATUS (*EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_POLL_IO_MEM)(
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *self, 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH width,
    uint64_t address,
    uint64_t mask,
    uint64_t value,
    uint64_t delay,
    uint64_t *result);

typedef EFI_STATUS (*EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_IO_MEM)(
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *self,
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH width,
    uint64_t address,
    uint64_t count,
    void *buffer);

struct EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS
{ 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_IO_MEM      read;
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_IO_MEM      write;
};

#define EFI_PCI_ATTRIBUTE_ISA_MOTHERBOARD_IO    0x0001
#define EFI_PCI_ATTRIBUTE_ISA_IO                0x0002
#define EFI_PCI_ATTRIBUTE_VGA_PALETTE_IO        0x0004
#define EFI_PCI_ATTRIBUTE_VGA_MEMORY            0x0008
#define EFI_PCI_ATTRIBUTE_VGA_IO                0x0010
#define EFI_PCI_ATTRIBUTE_IDE_PRIMARY_IO        0x0020
#define EFI_PCI_ATTRIBUTE_IDE_SECONDARY_IO      0x0040
#define EFI_PCI_ATTRIBUTE_MEMORY_WRITE_COMBINE  0x0080
#define EFI_PCI_ATTRIBUTE_MEMORY_CACHED         0x0800
#define EFI_PCI_ATTRIBUTE_MEMORY_DISABLE        0x1000
#define EFI_PCI_ATTRIBUTE_DUAL_ADDRESS_CYCLE    0x8000
#define EFI_PCI_ATTRIBUTE_ISA_IO_16             0x10000
#define EFI_PCI_ATTRIBUTE_VGA_PALETTE_IO_16     0x20000
#define EFI_PCI_ATTRIBUTE_VGA_IO_16             0x40000

typedef enum { 
    EfiPciOperationBusMasterRead, 
    EfiPciOperationBusMasterWrite, 
    EfiPciOperationBusMasterCommonBuffer, 
    EfiPciOperationBusMasterRead64, 
    EfiPciOperationBusMasterWrite64, 
    EfiPciOperationBusMasterCommonBuffer64, 
    EfiPciOperationMaximum
} EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION;

typedef EFI_STATUS (*EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_COPY_MEM)( 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *self,
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  width,
    uint64_t                               destAddress,
    uint64_t                               srcAddress,
    uint64_t                               count
);

typedef EFI_STATUS (*EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_MAP)( 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL            *self,
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION  operation,
    void                                       *hostAddress,    /* in */
    uint64_t                                   *numberOfBytes,  /* in/out */
    EFI_PHYSICAL_ADDRESS                       *deviceAddress,  /* out */
    void                                       **mapping        /* out */
);

typedef EFI_STATUS (*EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_UNMAP)( 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *self,
    void                             *mapping   /* in */
);

typedef EFI_STATUS(*EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ALLOCATE_BUFFER)(
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *self,
    EFI_ALLOCATE_TYPE               type,   /* this parameter is ignored */
    EFI_MEMORY_TYPE                 memoryType,
    uint64_t                        pages,
    void                            **hostAddress,
    uint64_t                        attributes
 );

typedef EFI_STATUS(*EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_FREE_BUFFER)( 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *self,
    uint64_t                        pages,
    void                            *hostAddress
);

typedef EFI_STATUS(*EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_FLUSH)( 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *self
);

typedef EFI_STATUS(*EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GET_ATTRIBUTES)( 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *self,
    uint64_t                        *supports   /* optional */, 
    uint64_t                        *attributes /* optional */ 
);

typedef EFI_STATUS(*EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_SET_ATTRIBUTES)( 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *self,
    uint64_t                        attributes,
    uint64_t                        *resourceBase  /*optional*/,
    uint64_t                        *resourceLength /* optional */ 
);

typedef EFI_STATUS(*EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_CONFIGURATION)( 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *self,
    void                            **Resources
);

struct EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL
{ 
    EFI_HANDLE ParentHandle;
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_POLL_IO_MEM     pollMem;
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_POLL_IO_MEM     pollIo;
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS          mem; 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS          io; 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS          pci; 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_COPY_MEM        copyMem; 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_MAP             map; 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_UNMAP           unmap;  
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ALLOCATE_BUFFER allocateBuffer; 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_FREE_BUFFER     freeBuffer; 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_FLUSH           flush; 
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GET_ATTRIBUTES  getAttributes;
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_SET_ATTRIBUTES  setAttributes;
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_CONFIGURATION   configuration;
    uint32_t                                        segmentNumber;
};



// ========================================================================
//
//   EFI_PCI_IO_PROTOCOL
//
// ========================================================================

struct EFI_PCI_IO_PROTOCOL; // pre-declaration
#define EFI_PCI_IO_PROTOCOL_GUID \ {0x4cf5b200,0x68b8,0x4ca5,\  {0x9e,0xec,0xb2,0x3e,0x3f,0x50,0x02,0x9a}}

typedef enum 
{ 
    EfiPciIoWidthUint8, 
    EfiPciIoWidthUint16, 
    EfiPciIoWidthUint32, 
    EfiPciIoWidthUint64, 
    EfiPciIoWidthFifoUint8, 
    EfiPciIoWidthFifoUint16, 
    EfiPciIoWidthFifoUint32, 
    EfiPciIoWidthFifoUint64, 
    EfiPciIoWidthFillUint8, 
    EfiPciIoWidthFillUint16, 
    EfiPciIoWidthFillUint32, 
    EfiPciIoWidthFillUint64, 
    EfiPciIoWidthMaximum
} EFI_PCI_IO_PROTOCOL_WIDTH;


typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_POLL_IO_MEM)(
    EFI_PCI_IO_PROTOCOL           *self,
    EFI_PCI_IO_PROTOCOL_WIDTH     width,
    uint8_t                       barIndex,
    uint64_t                      offset,
    uint64_t                      mask,
    uint64_t                      value,
    uint64_t                      delay,
    uint64_t                     *result
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_IO_MEM)( 
    EFI_PCI_IO_PROTOCOL         *self,
    EFI_PCI_IO_PROTOCOL_WIDTH   width,
    uint8_t                     barIndex,
    uint64_t                    offset,
    uint64_t                    count,
    void                        *buffer
);

typedef struct { 
    EFI_PCI_IO_PROTOCOL_IO_MEM read;
    EFI_PCI_IO_PROTOCOL_IO_MEM write;
} EFI_PCI_IO_PROTOCOL_ACCESS;

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_CONFIG)(
    EFI_PCI_IO_PROTOCOL         *self,
    EFI_PCI_IO_PROTOCOL_WIDTH   width,
    uint32_t                    offset,
    uint64_t                    count,
    void                       *buffer
);

#define EFI_PCI_IO_ATTRIBUTE_ISA_MOTHERBOARD_IO   0x0001
#define EFI_PCI_IO_ATTRIBUTE_ISA_IO               0x0002
#define EFI_PCI_IO_ATTRIBUTE_VGA_PALETTE_IO       0x0004
#define EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY           0x0008
#define EFI_PCI_IO_ATTRIBUTE_VGA_IO               0x0010
#define EFI_PCI_IO_ATTRIBUTE_IDE_PRIMARY_IO       0x0020
#define EFI_PCI_IO_ATTRIBUTE_IDE_SECONDARY_IO     0x0040
#define EFI_PCI_IO_ATTRIBUTE_MEMORY_WRITE_COMBINE 0x0080
#define EFI_PCI_IO_ATTRIBUTE_IO                   0x0100
#define EFI_PCI_IO_ATTRIBUTE_MEMORY               0x0200
#define EFI_PCI_IO_ATTRIBUTE_BUS_MASTER           0x0400
#define EFI_PCI_IO_ATTRIBUTE_MEMORY_CACHED        0x0800
#define EFI_PCI_IO_ATTRIBUTE_MEMORY_DISABLE       0x1000
#define EFI_PCI_IO_ATTRIBUTE_EMBEDDED_DEVICE      0x2000
#define EFI_PCI_IO_ATTRIBUTE_EMBEDDED_ROM         0x4000
#define EFI_PCI_IO_ATTRIBUTE_DUAL_ADDRESS_CYCLE   0x8000
#define EFI_PCI_IO_ATTRIBUTE_ISA_IO_16            0x10000
#define EFI_PCI_IO_ATTRIBUTE_VGA_PALETTE_IO_16    0x20000
#define EFI_PCI_IO_ATTRIBUTE_VGA_IO_16            0x40000

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_POLL_IO_MEM)( 
    EFI_PCI_IO_PROTOCOL        *self,
    EFI_PCI_IO_PROTOCOL_WIDTH  width,
    uint8_t                    barIndex,
    uint64_t                   offset,
    uint64_t                   mask,
    uint64_t                   value,
    uint64_t                   delay,
    uint64_t                  *result
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_POLL_IO_MEM)( 
    EFI_PCI_IO_PROTOCOL         *self,
    EFI_PCI_IO_PROTOCOL_WIDTH   width,
    int8_t                      barIndex,
    uint64_t                    offset,
    uint64_t                    mask,
    uint64_t                    value,
    uint64_t                    delay,
    uint64_t                   *result
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_MEM)(
    EFI_PCI_IO_PROTOCOL        *self,
    EFI_PCI_IO_PROTOCOL_WIDTH  width,
    uint8_t                    barIndex,
    uint64_t                   offset,
    uint64_t                   count,
    void                      *buffer
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_MEM)(
    EFI_PCI_IO_PROTOCOL        *This,
    EFI_PCI_IO_PROTOCOL_WIDTH  width,
    uint8_t                    barIndex,
    uint64_t                   offset,
    uint64_t                   count,
    void                      *buffer
);

typedef enum 
{ 
    EfiPciIoOperationBusMasterRead, 
    EfiPciIoOperationBusMasterWrite, 
    EfiPciIoOperationBusMasterCommonBuffer, 
    EfiPciIoOperationMaximum
} EFI_PCI_IO_PROTOCOL_OPERATION;

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_CONFIG)( 
    EFI_PCI_IO_PROTOCOL        *self,
    EFI_PCI_IO_PROTOCOL_WIDTH  width,
    uint32_t                   offset,
    uint64_t                   count,
    void                      *buffer
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_COPY_MEM)( 
    EFI_PCI_IO_PROTOCOL        *self,
    EFI_PCI_IO_PROTOCOL_WIDTH  width,
    uint8_t                    destBarIndex,
    uint64_t                   destOffset,
    uint8_t                    srcBarIndex,
    uint64_t                   srcOffset,
    uint64_t                   count
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_MAP)( 
    EFI_PCI_IO_PROTOCOL            *self,
    EFI_PCI_IO_PROTOCOL_OPERATION  operation,
    void                           *hostAddress, /* in */
    uint64_t                       *numberOfBytes, /* in out */
    EFI_PHYSICAL_ADDRESS           *deviceAddress, /* out */
    void                           **mapping /* out */
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_UNMAP)( 
    EFI_PCI_IO_PROTOCOL   *self,
    void                  *mapping
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_ALLOCATE_BUFFER)( 
    EFI_PCI_IO_PROTOCOL  *This,
    EFI_ALLOCATE_TYPE    type,  /* ignored */
    EFI_MEMORY_TYPE      memoryType,
    uint64_t             pages,
    void                 **hostAddress,
    uint64_t             attributes
);

struct EFI_PCI_IO_PROTOCOL_CONFIG_ACCESS
{   
    EFI_PCI_IO_PROTOCOL_CONFIG       read;
    EFI_PCI_IO_PROTOCOL_CONFIG       write;
};

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_FREE_BUFFER)(
    EFI_PCI_IO_PROTOCOL  *self,
    uint64_t              pages,
    void                 *hostAddress    /* in */
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_FLUSH)( 
    EFI_PCI_IO_PROTOCOL   *self
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_GET_LOCATION)( 
    EFI_PCI_IO_PROTOCOL *self,
    uint64_t               *segmentNumber,
    uint64_t               *busNumber,
    uint64_t               *deviceNumber,
    uint64_t               *functionNumber
);

typedef enum 
{ 
    EfiPciIoAttributeOperationGet, 
    EfiPciIoAttributeOperationSet, 
    EfiPciIoAttributeOperationEnable, 
    EfiPciIoAttributeOperationDisable, 
    EfiPciIoAttributeOperationSupported, 
    EfiPciIoAttributeOperationMaximum
} EFI_PCI_IO_PROTOCOL_ATTRIBUTE_OPERATION;

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_ATTRIBUTES)( 
    EFI_PCI_IO_PROTOCOL                     *This,
    EFI_PCI_IO_PROTOCOL_ATTRIBUTE_OPERATION operation,
    uint64_t                                  attributes,
    uint64_t                                 *result  /* OPTIONAL  */
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_GET_BAR_ATTRIBUTES)(
    EFI_PCI_IO_PROTOCOL  *self,
    uint8_t               barIndex,
    uint64_t             *supports  /* optional */, 
    void                 **Resources /* optional */
);

typedef EFI_STATUS(*EFI_PCI_IO_PROTOCOL_SET_BAR_ATTRIBUTES)(
    EFI_PCI_IO_PROTOCOL *self,
    uint64_t             Attributes,
    uint8_t              BarIndex,
    uint64_t             *Offset,    /* in out */
    uint64_t             *Length     /* in out */
); 

struct EFI_PCI_IO_PROTOCOL { 
    EFI_PCI_IO_PROTOCOL_POLL_IO_MEM        pollMem;
    EFI_PCI_IO_PROTOCOL_POLL_IO_MEM        pollIo;
    EFI_PCI_IO_PROTOCOL_ACCESS             mem;
    EFI_PCI_IO_PROTOCOL_ACCESS             io;
    EFI_PCI_IO_PROTOCOL_CONFIG_ACCESS      pci;
    EFI_PCI_IO_PROTOCOL_COPY_MEM           copyMem;
    EFI_PCI_IO_PROTOCOL_MAP                map;
    EFI_PCI_IO_PROTOCOL_UNMAP              unmap;
    EFI_PCI_IO_PROTOCOL_ALLOCATE_BUFFER    allocateBuffer;
    EFI_PCI_IO_PROTOCOL_FREE_BUFFER        freeBuffer;
    EFI_PCI_IO_PROTOCOL_FLUSH              flush;
    EFI_PCI_IO_PROTOCOL_GET_LOCATION       getLocation;
    EFI_PCI_IO_PROTOCOL_ATTRIBUTES         attributes;
    EFI_PCI_IO_PROTOCOL_GET_BAR_ATTRIBUTES fetBarAttributes;
    EFI_PCI_IO_PROTOCOL_SET_BAR_ATTRIBUTES setBarAttributes;
    uint64_t               romSize;
    void                  *romImage;
};
