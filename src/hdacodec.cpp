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

volatile uint8_t  HDACodec::read8(size_t offset) const
{
    return *(((volatile uint8_t*)m_base) + offset);
}

volatile uint16_t HDACodec::read16(size_t offset) const
{
    return *(volatile uint16_t*)(((uint8_t*)m_base) + offset);
}

volatile uint32_t HDACodec::read32(size_t offset) const 
{
    return *(volatile uint32_t*)(((uint8_t*)m_base) + offset);
}

volatile uint64_t HDACodec::read64(size_t offset) const
{
    return *(volatile uint64_t*)(((uint8_t*)m_base) + offset);    
}

void HDACodec::disableInterrupt()
{
    writeReg("intctl", 0);
}

void HDACodec::turnOffCorbRirbDmapos()
{
    writeReg("corbrp", 0);
    writeReg("rirbctl", 0);
    writeReg("dplbase", 0);
}

void HDACodec::inputStreamTurnOff()
{
    write32(m_inputDescriptor, 0x00200000);
}

void HDACodec::outputStreamTurnOff()
{
    write32(m_inputDescriptor, 0x00140000);
}

void HDACodec::inputStreamTurnOn()
{
    write32(m_inputDescriptor, 0x00140002);
}

void HDACodec::outputStreamTurnOn()
{
    write32(m_inputDescriptor, 0x00140002);
}

bool HDACodec::setOutputNode(uint32_t node)
{
    sendVerb(0, node, 0xF00, 0x12);  // get amplifier info
    auto result = readVerbResponse();
    if (!result.has_value())
        return false;

    auto maxVolume = result.value_or(0) & 0xFF;

    sendVerb(0, node, 0x706 /* set stream */, 0x10 /* stream 1 channel 1 */);
    result = readVerbResponse();
    if (!result.has_value())
        return false;
    
    sendVerb(0, node, 0x300 /* set volume */, maxVolume | 0xB000);
    result = readVerbResponse();
    if (!result.has_value())
        return false;

    return true;
}

void HDACodec::inputStreamSetBuffer(size_t address)
{
    write32(m_inputDescriptor+0x18, address);
    write32(m_inputDescriptor+0x18+4, 0);

    write32(m_inputDescriptor+0x0C, 1); // last valid index: two entries
}

void HDACodec::outputStreamSetBuffer(size_t address)
{
    write32(m_outputDescriptor+0x18, address);
    write32(m_outputDescriptor+0x18+4, 0);

    write32(m_outputDescriptor+0x0C, 1); // last valid index: two entries
}

void HDACodec::setOutputBuffer(void *data, size_t length)
{
    
}

void HDACodec::setSSync()
{
    
}

void HDACodec::outputStreamLength(size_t length)
{
    write32(m_outputDescriptor+0x08, length);
}

void HDACodec::outputStreamFormat(const StreamFormat &format)
{
    write32(m_outputDescriptor+0x12, format.m_value);
}

void HDACodec::playSound(void *data, size_t length, const StreamFormat &format)
{
    outputStreamSetBuffer((size_t)data);
    outputStreamLength(length);
    outputStreamFormat(format);
    outputStreamTurnOn();
}

void HDACodec::stopSound()
{
    outputStreamTurnOff();
}

void HDACodec::sendVerb(uint32_t codec, uint32_t node, uint32_t verb, uint32_t command)
{
    const uint32_t word = (codec << 28) | (node << 20) | (verb << 8) | command;
    
    writeReg("ics", 0x02);  // clear immediate command valid bit
    writeReg("icw", word);  // write immediate command
    writeReg("ics", 0x01);  // trigger a transmission of the command
}

std::optional<uint32_t> HDACodec::readVerbResponse()
{
    size_t retries = 1024;
    while((readReg("ics") & 0x03) != 0x02)
    {
        retries--;
        if (retries==0)
        {
            return std::nullopt;
        }
    }
    return readReg("irr");
}
