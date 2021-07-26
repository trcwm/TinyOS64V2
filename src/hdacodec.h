#pragma once

#include <cstdint>
#include <cstdlib>
#include <optional>
#include <array>

#include "helpers.h"
#include "efi.h"

extern size_t print(const char *fmt, ...);
extern EFI_SYSTEM_TABLE *g_sysTbl;

class HDACodec
{
public:
    HDACodec(uint32_t baseAddress) : m_base((void*)(uint64_t)baseAddress) 
    {
        m_descriptorBuffer = 0xFFFFFFFF;
        auto result = g_sysTbl->m_bootServices->m_allocatePages(AllocateMaxAddress,
            EfiRuntimeServicesData, 1, &m_descriptorBuffer);

        if (result != EFI_SUCCESS)
        {
            print("Failed to allocate memory for codec descriptor buffer!\n\r");
        }
        else
        {
            print("Allocated memory for codec buffer at 0x%x!\n\r", m_descriptorBuffer);
        }

        m_audioBuffer = 0xFFFFFFFF;
        auto result = g_sysTbl->m_bootServices->m_allocatePages(AllocateMaxAddress,
            EfiRuntimeServicesData, 16, &m_audioBuffer);

        if (result != EFI_SUCCESS)
        {
            print("Failed to allocate memory for codec audio buffer!\n\r");
        }
        else
        {
            print("Allocated memory for codec audio at 0x%x!\n\r", m_audioBuffer);
        }        
    }

    struct RegDef
    {
        size_t offset;
        size_t width;
        const char *name;
    };

    constexpr static std::array<RegDef, 42> regs =
    {{
        {0x00, 2, "gcap"},
        {0x02, 1, "vmin"},
        {0x03, 1, "vmax"},
        {0x04, 2, "outpay"},
        {0x08, 4, "gctl"},
        {0x0C, 2, "wakeen"},
        {0x0E, 2, "statests"},
        {0x10, 2, "gsts"},
        {0x18, 2, "outstrmpay"},
        {0x1A, 2, "instrmpay"},
        {0x20, 4, "intctl"},
        {0x24, 4, "intsts"},
        {0x30, 4, "wallclock"},
        {0x38, 4, "ssync"},
        {0x40, 4, "corblbase"},
        {0x44, 4, "corbubase"},
        {0x48, 2, "corbwp"},
        {0x4A, 2, "corbrp"},
        {0x4C, 1, "corbctl"},
        {0x4D, 1, "corbsts"},
        {0x4E, 1, "corbsize"},
        {0x50, 4, "rirblbase"},
        {0x54, 4, "rirbubase"},
        {0x58, 2, "rirbwp"},
        {0x5A, 2, "rintcnt"},
        {0x5C, 1, "rirbctl"},
        {0x5D, 1, "rirbsts"},
        {0x5E, 1, "rirbsize"},
        {0x70, 4, "dplbase"},
        {0x74, 4, "dpubase"},
        {0x80, 4, "sdnctl"},    // for lower 8 bits: software must use 0‟s for writes to bits.
        {0x83, 1, "sdnsts"},    // software must use 0‟s for writes to bits.
        {0x84, 4, "sdnlpib"},
        {0x88, 4, "sdncbl"},
        {0x8C, 4, "sdnlvi"},
        {0x90, 2, "sdnfifos"},
        {0x92, 2, "sdnfmt"},
        {0x98, 4, "sdnbdpl"},
        {0x9C, 2, "sdnbdpu"},
        {0x60, 4, "icw"},
        {0x64, 4, "irr"},
        {0x68, 2, "ics"}
    }};

    constexpr std::optional<RegDef> getReg(const std::string_view &regname) const
    {
        for(auto reg : regs)
        {
            if (strncmp(regname, reg.name))
                return reg;
        }

        return std::nullopt;
    }

    uint64_t readReg(const std::string_view &regname) const
    {
        auto reg = getReg(regname);
        if (reg.has_value())
        {
            switch(reg->width)
            {
            case 1:
                return read8(reg->offset);
            case 2:
                return read16(reg->offset);
            case 4:
                return read32(reg->offset);
            default:
                // error!
                break;
            }
        }
        print("ERROR: readReg cannot find register\n\r");
        return 0;
    }

    bool writeReg(const std::string_view &regname, uint64_t v)
    {        
        auto reg = getReg(regname);
        if (reg.has_value())
        {
            switch(reg->width)
            {
            case 1:
                write8(reg->offset, v);
                return true;
            case 2:
                write16(reg->offset, v);
                return true;
            case 4:
                write32(reg->offset, v);
                return true;
            default:
                // error!
                break;
            }
        }
        print("ERROR: writeReg cannot find register\n\r");
        return false;
    }

    void reset()
    {
        print("Codec reset .. \n\r");
        uint32_t gctl = readReg("gctl");
        writeReg("gctl", gctl & 0xFFFFFFFE); // enter reset

        // wait a bit.. 
        for(volatile uint32_t w=0; w<65535*10000; w++) {} // wait loop.

        // take controller out of reset
        print("Codec reset de-assert .. \n\r");
        gctl = readReg("gctl");
        writeReg("gctl", gctl | 1); // leave reset
        
        // wait until bit 0 of GCTL clears itself
        do
        {
            for(volatile uint32_t w=0; w<65535*1000; w++) {} // wait loop.
        } while((readReg("gctl") & 0x01) == 0);

        print("Codec reset complete .. \n\r");

        // setup the input/output descriptors
        auto capabilities = readReg("gcap");
        uint64_t numberOfOutputStreams = (capabilities >> 12) & 0xF;
        uint64_t numberOfInputStreams  = (capabilities >> 8) & 0xF;

        print("Codec reports %d input and %d output streams\n\r", numberOfInputStreams, numberOfOutputStreams);

        m_inputDescriptor  = 0x80;
        m_outputDescriptor = 0x80 + 0x20 * numberOfInputStreams;
    }

    struct StreamFormat
    {        
        uint32_t m_value;
    };

    void disableInterrupt();
    void turnOffCorbRirbDmapos();
    //void inputStreamTurnOff();
    void outputStreamTurnOff();
    //void inputStreamTurnOn();
    void outputStreamTurnOn();
    //void inputStreamSetBuffer(size_t address);

    void outputStreamSetDescriptorList();
    void outputStreamLength(size_t length);
    void outputStreamFormat(const StreamFormat &format);
    void setOutputAudioBuffer(void *data, size_t length);

    void setSSync();
    bool setOutputNode(uint32_t node);    

    void playSound(size_t length, const StreamFormat &format);
    void stopSound();

    void sendVerb(uint32_t codec, uint32_t node, uint32_t verb, uint32_t command);
    std::optional<uint32_t> readVerbResponse();

    void    write8(size_t offset, uint8_t v);
    void    write16(size_t offset, uint16_t v);
    void    write32(size_t offset, uint32_t v);
    void    write64(size_t offset, uint64_t v);

    volatile uint8_t  read8(size_t offset) const;
    volatile uint16_t read16(size_t offset) const;
    volatile uint32_t read32(size_t offset) const; 
    volatile uint64_t read64(size_t offset) const;

protected:
    EFI_PHYSICAL_ADDRESS m_descriptorBuffer;
    EFI_PHYSICAL_ADDRESS m_audioBuffer;
    size_t  m_inputDescriptor;
    size_t  m_outputDescriptor;

    void *m_base;
};