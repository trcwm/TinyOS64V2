#pragma once

struct AudioDeviceResult
{
    uint8_t     m_bus;
    uint8_t     m_device;
    uint8_t     m_func;
    uint8_t     m_intPin;
    uint8_t     m_intLine;
    uint32_t    m_address;
    bool        m_valid;
};

AudioDeviceResult findHDAudioDevice();
