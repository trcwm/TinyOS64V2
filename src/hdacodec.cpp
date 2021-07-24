#include "hdacodec.h"

void HDACodec::write8(size_t offset, uint8_t v)
{
    *(((volatile uint8_t*)m_base) + offset) = v;
}

void HDACodec::write16(size_t offset, uint16_t v)
{
    *(volatile uint16_t*)(((uint8_t*)m_base) + offset) = v;
}

void HDACodec::write32(size_t offset, uint32_t v)
{
    *(volatile uint32_t*)(((uint8_t*)m_base) + offset) = v;
}

void HDACodec::write64(size_t offset, uint64_t v)
{
    *(volatile uint64_t*)(((uint8_t*)m_base) + offset) = v;
}

volatile uint8_t  HDACodec::read8(size_t offset)
{
    return *(((volatile uint8_t*)m_base) + offset);
}

volatile uint16_t HDACodec::read16(size_t offset)
{
    return *(volatile uint16_t*)(((uint8_t*)m_base) + offset);
}

volatile uint32_t HDACodec::read32(size_t offset)
{
    return *(volatile uint32_t*)(((uint8_t*)m_base) + offset);
}

volatile uint64_t HDACodec::read64(size_t offset)
{
    return *(volatile uint64_t*)(((uint8_t*)m_base) + offset);    
}
