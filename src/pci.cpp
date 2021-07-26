
#include <cstdlib>
#include <cstdint>
#include "pci.h"

extern size_t print(const char *fmt, ...);

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
}

// see: https://wiki.osdev.org/PCI
static uint32_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
            (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl(0xCF8, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    //tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    return inl(0xCFC);
}

static void pciConfigWriteWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
            (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    
    /* write out the address */
    outl(0xCF8, address);

    /* write out the data */
    outl(0xCFC, value);
}

void setPCICommandReg(uint8_t bus, uint8_t device, uint8_t func, uint16_t value)
{
    uint32_t reg = pciConfigReadWord(bus, device, func, 0x04);
    reg &= 0xFFFF0000;
    reg |= (uint32_t)value;
    pciConfigWriteWord(bus, device, func, 0x04, reg);
    reg = pciConfigReadWord(bus, device, func, 0x04);
    print("PCI command reg readback: %x\n\r", reg);
}

AudioDeviceResult findHDAudioDevice(
    const uint8_t busStart, 
    const uint8_t deviceStart, 
    const uint8_t funcStart)
{
    AudioDeviceResult result;
    result.m_valid = false;

    for(uint8_t bus=busStart; bus != 0xff; bus++)
    {
        for(uint8_t device=deviceStart; device<32; device++)
        {
            for(uint8_t func=funcStart; func<8; func++)
            {
                auto IDs = pciConfigReadWord(bus,device,func,0);
                auto vendorID = IDs & 0xFFFF;
                auto deviceID = IDs >> 16;
                
                if (vendorID != 0xFFFF)
                {
                    //print(L"bus %d - device %d - func %d -> vendorID: %x deviceID: %x\n\r",
                    //    bus,device,func, vendorID, deviceID);

                    auto reg02 = pciConfigReadWord(bus,device,func, 8);
                    auto classCode    = (reg02>> 24)  & 0xFF;
                    auto subclassCode = (reg02 >> 16)  & 0xFF;

                    if ((classCode == 0x04) && (subclassCode == 0x03))
                    {
                        // found an HD audio device
                        print("HD Audio device found on bus %d device %d func %d [%x:%x]\n\r",
                            bus, device, func, vendorID, deviceID);

                        result.m_address = pciConfigReadWord(bus, device, func, 0x10);

                        print("  base address #0: %x\n\r", result.m_address);
                        print("  base address #1: %x\n\r", pciConfigReadWord(bus, device, func, 0x14));
                        print("  base address #2: %x\n\r", pciConfigReadWord(bus, device, func, 0x18));
                        print("  base address #3: %x\n\r", pciConfigReadWord(bus, device, func, 0x1C));
                        print("  base address #4: %x\n\r", pciConfigReadWord(bus, device, func, 0x20));
                        print("  base address #5: %x\n\r", pciConfigReadWord(bus, device, func, 0x24));

                        auto reg0F = pciConfigReadWord(bus,device,func, 0x3C);

                        auto intPin  = (reg0F >> 8) & 0xFF;
                        auto intLine = reg0F & 0xFF;

                        print("  int line: %d   - int pin: %d\n\r", intPin, intLine);

                        result.m_valid = true;
                        result.m_bus = bus;
                        result.m_device = device;
                        result.m_func = func;
                        result.m_intLine = intLine;
                        result.m_intPin = intPin;

                        result.m_deviceId = deviceID;
                        result.m_vendorId = vendorID;
                        return result;
                    }
                }
            }
        }
    }

    return result;
}
