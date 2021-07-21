#include <cstdint>
#include <uchar.h>
#include <array>
#include "myprint.h"
#include "efi.h"

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
    void efi_init(EFI_HANDLE handle, EFI_SYSTEM_TABLE *sysTbl)
    {
        sysTbl->m_conOut->m_clearScreen(sysTbl->m_conOut);
        sysTbl->m_conOut->m_enableCursor(sysTbl->m_conOut, 1);

        const wchar_t*str = L"Hello, world!\n\r";
        sysTbl->m_conOut->m_outputString(sysTbl->m_conOut, (wchar_t*)str);
        
        // try to find the Graphics driver
        UEFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
        EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

        if (sysTbl->m_bootServices->m_locateProtocol(&gopGuid, nullptr, (void**)&gop) != 0)
        {
            sysTbl->m_conOut->m_outputString(sysTbl->m_conOut, (CHAR16*) L"Shit");
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
            snprintf(buffer, sizeof(buffer), L"Current video mode is %d\n\r", currentMode);
            sysTbl->m_conOut->m_outputString(sysTbl->m_conOut, buffer);

            snprintf(buffer, sizeof(buffer), L"Frame buffer address  %X\n\r", dispAddr);
            sysTbl->m_conOut->m_outputString(sysTbl->m_conOut, buffer);

            snprintf(buffer, sizeof(buffer), L"Frame buffer size     %d bytes\n\r", dispSize);
            sysTbl->m_conOut->m_outputString(sysTbl->m_conOut, buffer);

            snprintf(buffer, sizeof(buffer), L"Available modes: %d\n\r", gop->Mode->m_maxMode);
            sysTbl->m_conOut->m_outputString(sysTbl->m_conOut, buffer);

            // try set mode to 640x480
            int32_t desiredMode = -1;
            for(size_t modeIdx = 0; modeIdx < maxMode; modeIdx++)
            {
                if (modeIdx % 2 == 0)
                {
                    sysTbl->m_conOut->m_outputString(sysTbl->m_conOut, (CHAR16*) L"\n\r");
                }

                gop->QueryMode(gop, modeIdx, &sizeOfInfo, &info);

                auto modeWidth  = info->m_horizontalResolution;
                auto modeHeight = info->m_verticalResolution;

                snprintf(buffer, sizeof(buffer), L"mode %d -> x=%d  y=%d        ", 
                    modeIdx,
                    modeWidth,
                    modeHeight);
                
                sysTbl->m_conOut->m_outputString(sysTbl->m_conOut, buffer);

                if ((modeWidth == 640) && (modeHeight== 480))
                {
                    desiredMode = modeIdx;
                }
            }
            sysTbl->m_conOut->m_outputString(sysTbl->m_conOut, (CHAR16*) L"\n\r");

            if (desiredMode != -1)
            {
                snprintf(buffer, sizeof(buffer), L"Setting GOP to mode %d\n", desiredMode);
                sysTbl->m_conOut->m_outputString(sysTbl->m_conOut, buffer);
                gop->SetMode(gop, desiredMode);
            }

            mandelbrot(gop->Mode->m_info->m_horizontalResolution, 
                gop->Mode->m_info->m_verticalResolution, (uint32_t*)dispAddr);
        }

        while(1) {}
    }
}
