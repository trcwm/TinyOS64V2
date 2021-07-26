#pragma once

struct AudioDeviceResult
{
    uint8_t     m_bus;
    uint8_t     m_device;
    uint8_t     m_func;

    uint8_t     m_intPin;
    uint8_t     m_intLine;

    uint16_t    m_vendorId;
    uint16_t    m_deviceId;

    uint32_t    m_address;
    bool        m_valid;
};

AudioDeviceResult findHDAudioDevice(
    const uint8_t busStart, 
    const uint8_t deviceStart, 
    const uint8_t funcStart    
);

void setPCICommandReg(uint8_t bus, uint8_t device, uint8_t func, uint16_t value);