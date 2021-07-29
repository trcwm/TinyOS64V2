#include <cstdint>
#include <uchar.h>
#include <array>
#include <vector>
#include <list>
#include <cstdarg>

#include "myprint.h"
#include "efi.h"
#include "acpi.h"
#include "pci.h"
#include "hdacodec.h"
#include "commands.h"
#include "allocator.h"

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

EFI_SYSTEM_TABLE *g_sysTbl;

size_t print(const char *fmt, ...)
{
    wchar_t buffer[256];

    va_list va;
    va_start(va,fmt);
    size_t count = snprintf(buffer, sizeof(buffer), fmt, va);
    va_end(va);
    g_sysTbl->m_conOut->m_outputString(g_sysTbl->m_conOut, buffer);
    return count;
}

EFI_STATUS waitForKey()
{
    uint64_t index;
    return g_sysTbl->m_bootServices->m_waitforEvent(1, &g_sysTbl->m_conIn->WaitForKey, &index);
}

EFI_STATUS readKey(EFI_INPUT_KEY &key)
{
    return g_sysTbl->m_conIn->ReadKeyStroke(g_sysTbl->m_conIn, &key);
}

void setHighResolutionTextMode()
{
    uint64_t bestMode = 0;
    uint64_t bestRows = 0;
    uint64_t bestCols = 0;

    uint64_t mode = 0;
    uint64_t cols = 0;
    uint64_t rows = 0;

    auto status = g_sysTbl->m_conOut->m_queryMode(g_sysTbl->m_conOut, mode, &cols, &rows);
    while(status == EFI_SUCCESS)
    {
        if (rows > bestRows)
        {
            bestRows = rows;
            bestCols = cols;
            bestMode = mode;
        }
        mode++;
        status = g_sysTbl->m_conOut->m_queryMode(g_sysTbl->m_conOut, mode, &cols, &rows);
    }

    g_sysTbl->m_conOut->m_setMode(g_sysTbl->m_conOut, bestMode);
    g_sysTbl->m_conOut->m_clearScreen(g_sysTbl->m_conOut);
    g_sysTbl->m_conOut->m_enableCursor(g_sysTbl->m_conOut, 1);

    print("Console: %d x %d\n\r", bestCols, bestRows);
}

extern "C"
{

    void do_graphics()
    {
        // try to find the Graphics driver
        UEFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
        EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

        if (g_sysTbl->m_bootServices->m_locateProtocol(&gopGuid, nullptr, (void**)&gop) != 0)
        {
            print("Shit");
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
            print("Current video mode is %d\n\r", currentMode);
            print("Frame buffer address  %X\n\r", dispAddr);
            print("Frame buffer size     %d bytes\n\r", dispSize);
            print("Available modes: %d\n\r", gop->Mode->m_maxMode);

            // try set mode to 640x480
            int32_t desiredMode = -1;
            for(size_t modeIdx = 0; modeIdx < maxMode; modeIdx++)
            {
                if (modeIdx % 2 == 0)
                {
                    print("\n\r");
                }

                gop->QueryMode(gop, modeIdx, &sizeOfInfo, &info);

                auto modeWidth  = info->m_horizontalResolution;
                auto modeHeight = info->m_verticalResolution;

                print("mode %d -> x=%d  y=%d        ", 
                    modeIdx,
                    modeWidth,
                    modeHeight);
    
                if ((modeWidth == 640) && (modeHeight== 480))
                {
                    desiredMode = modeIdx;
                }
            }

            print("\n\r");

            if (desiredMode != -1)
            {
                print("Setting GOP to mode %d\n", desiredMode);
                gop->SetMode(gop, desiredMode);
            }

            mandelbrot(gop->Mode->m_info->m_horizontalResolution, 
                gop->Mode->m_info->m_verticalResolution, (uint32_t*)dispAddr);
        }
    }


    class TestObject
    {
    public:
        TestObject()
        {
            print("Test object constructor called!\n\r");
        }

        uint32_t func()
        {
            return 123;
        }
    };

    void efi_init(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *sysTbl)
    {
        g_sysTbl = sysTbl;
        
        setHighResolutionTextMode();
        print("Hello, world! " __DATE__ " " __TIME__ "\n\r"); 

        g_sysTbl->m_bootServices->m_setWatchdogTimer(0, 0, 0, NULL);
        print("Watchdog disabled!\n\r");

        //auto obj = new TestObject();

        std::vector<TestObject> m_test;
        m_test.resize(3);

        m_test.emplace_back(TestObject());

        print("Number of items in the vector = %d\n\r", m_test.size());

        for(auto o : m_test)
        {
            print("Item %d\n\r", o.func());
        }

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

        auto HDADeviceInfo = findHDAudioDevice(0,0,0);
        AudioDeviceResult audioDevice;

        while(HDADeviceInfo.m_valid)
        {
            audioDevice = HDADeviceInfo;
            HDADeviceInfo = findHDAudioDevice(
                HDADeviceInfo.m_bus,
                HDADeviceInfo.m_device,
                HDADeviceInfo.m_func+1);
        }
        
        // set the command register
        setPCICommandReg(audioDevice.m_bus, audioDevice.m_device, audioDevice.m_func, 0x0406);

        print("Using codec address %x\n\r", audioDevice.m_address);
        HDACodec codec(audioDevice.m_address);

        uint64_t gcap = codec.readReg("gcap");
        auto outStreams = (gcap >> 12) & 0xF;
        auto inStreams  = (gcap >> 8)  & 0xF;
        auto bidiStreams= (gcap >> 3)  & 0x1F;
        auto dso = (gcap >> 1)  & 0x3;
        auto has64 = gcap & 0x1;

        print("  output streams: %d\n\r", outStreams);
        print("  input  streams: %d\n\r", inStreams);
        print("  bidi   streams: %d\n\r", bidiStreams);
        print("  SDO           : %d\n\r", dso);
        print("  64-bit        : %s\n\r", (has64 ? "YES" : "NO"));

        print("HDA version = %d.%d\n\r", codec.read8(0x03), codec.read8(0x02));
        print("GCTL        = %x\n\r", codec.readReg("gctl"));
        print("INTCTL      = %x\n\r", codec.readReg("intctl"));

        // setup WAKEEN so that all attached codecs 
        codec.writeReg("wakeen", 0x7F);

        // see Chapter 4 of HDA spec.
        codec.reset();
        codec.disableInterrupt();
        codec.turnOffCorbRirbDmapos();
        codec.outputStreamTurnOff();

        // check STATESTS reg for available codecs
        auto statests = codec.readReg("statests");
        for(uint8_t bit=0; bit<15; bit++)
        {
            if (((statests >> bit) & 0x01) == 1)
            {
                print("SDATA_IN[%d] has something connected\n\r", bit);
            }
        }

        print("ICS = %x\n\r", codec.readReg("ics"));

        uint32_t codecID = 0;
        codec.sendVerb(0,0,0xF00,0);
        auto result = codec.readVerbResponse();
        if (result.has_value())
        {
            codecID = result.value_or(0);
            print("Codec ID = %x\n\r", codecID);
        }
        else
        {
            print("Error reading codec number\n\r");
        }

        // see 7.3.4.6 of HDA spec
        uint32_t codecNum = 0;
        uint32_t outputNode = 2;    // hard code for now.
        
        if (!codec.setOutputNode(outputNode))
        {
            print("SetOutputNode failed!\n\r");
        }
        else
        {
            print("SetOutputNode OK!\n\r");
        }

        HDACodec::StreamFormat streamFormat;
        streamFormat.m_value = (1<<4) /* 16 bits packed */ | 1 /* stereo */;
        
        codec.outputStreamSetDescriptorList();
        codec.outputStreamLength(16*4096);
        codec.outputStreamFormat(streamFormat);
        codec.playSound(16*4096, streamFormat);

        size_t consoleBufferIdx = 0;
        char consoleBuffer[256];
        Commands commands(codec);

        print(">");
        while(1)
        {
            // wait for keypress;
            uint64_t index;
            auto status = waitForKey();
            if (status == EFI_SUCCESS)
            {
                EFI_INPUT_KEY key;
                status = readKey(key);

                if (status != EFI_SUCCESS)
                    continue;

                wchar_t buffer[3] = {key.m_unicodeChar, 0, 0};
                if (key.m_unicodeChar == 13)
                {
                    print("\n\r");
                    
                    // process command!
                    consoleBuffer[consoleBufferIdx] = 0;

                    if (!commands.execute(std::string_view(consoleBuffer, consoleBufferIdx)))
                    {
                        print("Command error\n\r");
                    }

                    consoleBufferIdx = 0;

                    print(">");
                }
                else
                {
                    g_sysTbl->m_conOut->m_outputString(g_sysTbl->m_conOut, buffer);
                    if (consoleBufferIdx < (sizeof(consoleBuffer)-1))
                    {
                        if ((key.m_unicodeChar < 256) && (key.m_unicodeChar >= 32))
                        {
                            consoleBuffer[consoleBufferIdx++] = (char)key.m_unicodeChar;
                        }
                        else if (key.m_unicodeChar == 0x08 /* backspace */)
                        {
                            if (consoleBufferIdx > 0)
                                consoleBufferIdx--;
                        }
                    }
                }
            }
        }
    }
}
