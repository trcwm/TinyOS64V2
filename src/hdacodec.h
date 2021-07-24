#pragma once

#include <cstdint>
#include <cstdlib>

extern size_t print(const char *fmt, ...);

class HDACodec
{
public:
    static constexpr size_t GCTL32      = 0x08;
    static constexpr size_t WAKEEN16    = 0x0C;
    static constexpr size_t STATESTS16  = 0x0E;

    HDACodec(uint32_t baseAddress) : m_base((void*)(uint64_t)baseAddress) {}

    void toggleReset()
    {
        print("Codec reset .. \n\r");
        uint32_t gctl = read32(GCTL32);
        write32(GCTL32, gctl & 0xFFFFFFFE); // enter reset

        // wait a bit.. 
        for(volatile uint32_t w=0; w<65535*10000; w++) {} // wait loop.

        // take controller out of reset
        print("Codec reset de-assert .. \n\r");
        gctl = read32(GCTL32);
        write32(GCTL32, gctl | 1); // leave reset
        
        // wait until bit 0 of GCTL clears itself
        do
        {
            for(volatile uint32_t w=0; w<65535*1000; w++) {} // wait loop.
        } while((read32(GCTL32) & 0x01) == 0);

        print("Codec reset complete .. \n\r");
    }

    void    write8(size_t offset, uint8_t v);
    void    write16(size_t offset, uint16_t v);
    void    write32(size_t offset, uint32_t v);
    void    write64(size_t offset, uint64_t v);

    volatile uint8_t  read8(size_t offset);
    volatile uint16_t read16(size_t offset);
    volatile uint32_t read32(size_t offset);
    volatile uint64_t read64(size_t offset);

protected:
    void *m_base;
};