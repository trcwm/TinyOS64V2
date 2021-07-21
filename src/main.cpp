#include <cstdint>
#include <uchar.h>
#include <array>
#include <cstdarg>

#include "myprint.h"
#include "efi.h"
#include "acpi.h"

bool operator==(const UEFI_GUID &id1, const UEFI_GUID &id2)
{
    if (id1.Data1 != id2.Data1) return false;
    if (id1.Data2 != id2.Data2) return false;
    if (id1.Data3 != id2.Data3) return false;
    for(size_t i=0; i<8; i++)
    {
        if (id1.Data4[i] != id2.Data4[i])
            return false;
    }
    return true;
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t ret;
    asm volatile ( "inl %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline void outl(uint16_t port, uint32_t val)
{
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static constexpr float absolute(float v)
{
    if (v < 0)
        return -v;

    return v;
}

// rescale by taking the square root
static constexpr float mysqrt(float x)
{
    int maxiter = 20;
    float guess = x;

    float eps = absolute(guess*guess - x);
    while(eps > 1e-10f)
    {
        guess = (guess + x/guess)/2.0f;
        maxiter--;
        
        if (maxiter == 0)
            return guess;

        eps = absolute(guess*guess - x);
    }
    
    return guess;
}

static constexpr std::array<uint32_t, 256> mktable()
{
    std::array<uint32_t, 256> tbl{};

    for(int i=0; i<tbl.size(); i++)
    {
        const uint8_t v = static_cast<uint8_t>(mysqrt(i*255));
        const uint32_t vv = (v << 24) | (v << 16) | ( v << 8) | v;
        tbl.at(i) = vv;
    }

    return tbl;
}

uint32_t rescale(const uint8_t v)
{
    const constexpr auto rescale_tbl = mktable();
    return rescale_tbl[v];
}

struct MyComplex
{
    constexpr MyComplex operator+(const MyComplex &other) const noexcept
    {
        return MyComplex{m_x + other.m_x, m_y + other.m_y};
    }

    constexpr MyComplex operator-(const MyComplex &other) const noexcept
    {
        return MyComplex{m_x - other.m_x, m_y - other.m_y};
    }

    constexpr MyComplex operator*(const MyComplex &other) const noexcept
    {
        return MyComplex{
            m_x*other.m_x - m_y*other.m_y, 
            m_x*other.m_y + m_y*other.m_x};
    }

    constexpr float mag2() const noexcept
    {
        return m_x*m_x + m_y*m_y;
    }

    float m_x, m_y;
};

void mandelbrot(const int width, const int height, uint32_t *frameBuffer)
{
    constexpr int maxiter = 255;

    for(int y=0; y<height; y++)
    {
        for(int x=0; x<width; x++)
        {
            const MyComplex c = {
                -1.5f + 2.0f*x / static_cast<float>(width), 
                -1.0f + 2.0f*y / static_cast<float>(height)};

            int iteration = 0;
            MyComplex z = {0.0f,0.0f};
            while(iteration < maxiter)
            {
                z = z*z + c;
                if (z.mag2() >= 4.0f)
                {
                    break;  // exit the loop
                }
                iteration++;
            }
            frameBuffer[x+width*y] = rescale(iteration);
        }
    }
}


extern "C"
{
    EFI_SYSTEM_TABLE *g_sysTbl;

    bool strncmp(const char *s1, const char *s2, size_t bytes)
    {
        for(size_t i=0; i<bytes; i++)
        {
            if (s1[i] != s2[i])
                return false;
            
            // detect early end of strings
            if ((s1[i] == 0) && (s2[i] == 0))
                return true;
        }
        return true;
    }

    size_t print(const wchar_t *fmt, ...)
    {
        wchar_t buffer[256];

        va_list va;
        va_start(va,fmt);
        size_t count = snprintf(buffer, sizeof(buffer), fmt, va);
        va_end(va);
        g_sysTbl->m_conOut->m_outputString(g_sysTbl->m_conOut, buffer);
        return count;
    }

    uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
    {
        uint32_t address;
        uint32_t lbus  = (uint32_t)bus;
        uint32_t lslot = (uint32_t)slot;
        uint32_t lfunc = (uint32_t)func;
        uint16_t tmp = 0;
    
        /* create configuration address as per Figure 1 */
        address = (uint32_t)((lbus << 16) | (lslot << 11) |
                (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    
        /* write out the address */
        outl(0xCF8, address);
        /* read in the data */
        /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
        tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
        return (tmp);
    }

    void do_graphics()
    {
        // try to find the Graphics driver
        UEFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
        EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

        if (g_sysTbl->m_bootServices->m_locateProtocol(&gopGuid, nullptr, (void**)&gop) != 0)
        {
            print(L"Shit");
        }
        else
        {
            EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
            size_t sizeOfInfo;
            auto currentMode = gop->Mode==NULL ? 0:gop->Mode->m_mode;
            auto maxMode     = gop->Mode==NULL ? 0:gop->Mode->m_maxMode;
            auto dispAddr    = gop->Mode==NULL ? 0:gop->Mode->m_frameBufferBase;
            uint64_t dispSize = gop->Mode==NULL ? 0:gop->Mode->m_frameBufferSize;

            wchar_t buffer[100];
            print(L"Current video mode is %d\n\r", currentMode);
            print(L"Frame buffer address  %X\n\r", dispAddr);
            print(L"Frame buffer size     %d bytes\n\r", dispSize);
            print(L"Available modes: %d\n\r", gop->Mode->m_maxMode);

            // try set mode to 640x480
            int32_t desiredMode = -1;
            for(size_t modeIdx = 0; modeIdx < maxMode; modeIdx++)
            {
                if (modeIdx % 2 == 0)
                {
                    print(L"\n\r");
                }

                gop->QueryMode(gop, modeIdx, &sizeOfInfo, &info);

                auto modeWidth  = info->m_horizontalResolution;
                auto modeHeight = info->m_verticalResolution;

                print(L"mode %d -> x=%d  y=%d        ", 
                    modeIdx,
                    modeWidth,
                    modeHeight);
    
                if ((modeWidth == 640) && (modeHeight== 480))
                {
                    desiredMode = modeIdx;
                }
            }

            print(L"\n\r");

            if (desiredMode != -1)
            {
                print(L"Setting GOP to mode %d\n", desiredMode);
                gop->SetMode(gop, desiredMode);
            }

            mandelbrot(gop->Mode->m_info->m_horizontalResolution, 
                gop->Mode->m_info->m_verticalResolution, (uint32_t*)dispAddr);
        }
    }



    void efi_init(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *sysTbl)
    {
        g_sysTbl = sysTbl;

        sysTbl->m_conOut->m_clearScreen(sysTbl->m_conOut);
        sysTbl->m_conOut->m_enableCursor(sysTbl->m_conOut, 1);

        print(L"Hello, world!\n\r");        

#if 0
        EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
        EFI_STATUS status;
        UEFI_GUID loadedImageProtocol = EFI_LOADED_IMAGE_PROTOCOL_GUID;
        status = g_sysTbl->m_bootServices->m_handleProtocol(imageHandle, 
            &loadedImageProtocol, (void**)&loadedImage);

        auto poolAllocMemType = loadedImage->ImageDataType;

        if (status == EFI_SUCCESS)
        {
            print(L"HandleProtocol OK!\n\r");
            print(L"Pool mem type = %X\n\r", poolAllocMemType);            
        }
        else
        {
            print(L"Barf code = %X!\n\r", status);
        }

        EFI_MEMORY_DESCRIPTOR       *EfiMemoryMap;
        uint64_t                    EfiMemoryMapSize;
        uint64_t                    EfiMapKey;
        uint64_t                    EfiDescriptorSize;
        uint32_t                    EfiDescriptorVersion;
        
        //
        // Get the EFI memory map.
        //
        EfiMemoryMapSize  = 0;
        EfiMemoryMap      = NULL;
        status = g_sysTbl->m_bootServices->m_getMemoryMap (
                        &EfiMemoryMapSize,
                        EfiMemoryMap,
                        &EfiMapKey,
                        &EfiDescriptorSize,
                        &EfiDescriptorVersion
                        );

        // No return check: we expect this to fail.
        print(L"Descriptor size = %d bytes\n\r", EfiDescriptorSize);
        print(L"Memorymap  size = %d bytes\n\r", EfiMemoryMapSize);

        //
        // Use size returned for the AllocatePool.
        //
        uint64_t allocateSize = EfiMemoryMapSize + 2 * EfiDescriptorSize;
        status = g_sysTbl->m_bootServices->m_allocatePool(poolAllocMemType, allocateSize, (void**) &EfiMemoryMap);

        if (status == EFI_SUCCESS)
        {
            print(L"Allocate Pool OK!\n\r");
        }
        else
        {
            print(L"Allocate Pool barf code = %X!\n\r", status);
        }

        status = g_sysTbl->m_bootServices->m_getMemoryMap (
                        &allocateSize,
                        EfiMemoryMap,
                        &EfiMapKey,
                        &EfiDescriptorSize,
                        &EfiDescriptorVersion
                        );

        if (status == EFI_SUCCESS)
        {
            print(L"getMemoryMap OK!\n\r");
        }
        else
        {
            print(L"getMemoryMap Barf code = %X!\n\r", status);
        }

        print(L"Memory descriptor size = %d\n\r" , sizeof(EFI_MEMORY_DESCRIPTOR));
        print(L"Reported          size = %d\n\r" , EfiDescriptorSize);

        const wchar_t *memory_types[] = 
        {
            L"EfiReservedMemoryType",
            L"EfiLoaderCode",
            L"EfiLoaderData",
            L"EfiBootServicesCode",
            L"EfiBootServicesData",
            L"EfiRuntimeServicesCode",
            L"EfiRuntimeServicesData",
            L"EfiConventionalMemory",
            L"EfiUnusableMemory",
            L"EfiACPIReclaimMemory",
            L"EfiACPIMemoryNVS",
            L"EfiMemoryMappedIO",
            L"EfiMemoryMappedIOPortSpace",
            L"EfiPalCode",
        };

        size_t count = 0;
        uint8_t *ptr = (uint8_t*)EfiMemoryMap;
        uint8_t *endPtr = ptr + EfiMemoryMapSize;
        do
        {
            count++;
            EFI_MEMORY_DESCRIPTOR *desc =  (EFI_MEMORY_DESCRIPTOR *)ptr;
            print(L"**type           = 0x%X %s\n\r", desc->m_type, memory_types[desc->m_type & 0xF]);
            print(L"  physical start = 0x%X\n\r", desc->m_physicalStart);
            print(L"  virtual  start = 0x%X\n\r", desc->m_virtualStart);
            print(L"  pages          = %d\n\r", desc->m_numberOfPages);
            print(L"  attributes     = 0x%X\n\r", desc->m_attribute);
            ptr += EfiDescriptorSize;
            
            if ((count % 4) == 3)
            {                
                //print(L"...\n\r");
                uint64_t index;
                auto status = g_sysTbl->m_bootServices->m_waitforEvent(1, &g_sysTbl->m_conIn->WaitForKey, &index);

                EFI_INPUT_KEY key;
                g_sysTbl->m_conIn->ReadKeyStroke(g_sysTbl->m_conIn, &key);
            }
        } while (ptr <= endPtr);
        

        //if (EFI_ERROR (Status)) {
        //    FreePool (EfiMemoryMap);
        //}
#endif

#if 0        
        constexpr UEFI_GUID ACPITABLEID = ACPI_TABLE_GUID;
        const ACPI::RSDP *rsdp = nullptr;

        for(size_t i=0; i<g_sysTbl->m_numberOfTableEntries; i++)
        {
            
            if (g_sysTbl->m_configurationTable[i].m_vendorGuid == ACPITABLEID)
            {
                print(L"Found ACPI 2.0 table at address 0x%X\n\r", g_sysTbl->m_configurationTable[i].m_vendorTable);
                rsdp = (const ACPI::RSDP*) g_sysTbl->m_configurationTable[i].m_vendorTable;
            }            
        }

        if (rsdp != nullptr)
        {
            if (strncmp(rsdp->m_signature, "RSD PTR ", 8))
            {
                print(L"RSDP signature found!\n\r");
            }

            ACPI::RSDT *rsdt = (ACPI::RSDT *)(uint64_t)rsdp->m_rsdtAddress;
            auto entries = (rsdt->m_header.m_length - sizeof(rsdt->m_header)) / 4;

            for(size_t i=0; i<entries; i++)
            {
                ACPI::ISDTHeader *header = (ACPI::ISDTHeader *)(uint64_t)rsdt->m_sdtPointers[i];
                print(L"  ID : ");
                for(size_t i=0; i<sizeof(header->m_signature); i++)
                {
                    print(L"%c", (wchar_t)header->m_signature[i]);
                }
                print(L"\n\r");
            }
        }
#endif

        for(uint8_t bus=0; bus != 0xff; bus++)
        {
            for(uint8_t device=0; device<32; device++)
            {
                for(uint8_t func=0; func<8; func++)
                {
                    auto vendorID = pciConfigReadWord(bus,device,func,0);
                    auto deviceID = pciConfigReadWord(bus,device,func,2);                    
                    if (vendorID != 0xFFFF)
                    {
                        print(L"bus %d - device %d - func %d -> vendorID: %x deviceID: %x\n\r",
                            bus,device,func, vendorID, deviceID);

                        auto classCode = pciConfigReadWord(bus,device,func, 10);
                        print(L"  class: %d - subclass: %d\n\r", (classCode >> 8) & 0xFF, classCode & 0xFF);
                    }
                }
            }
        }
        
        while(1)
        {

        }
    }
}
