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

bool HDACodec::disablePinWidget(HDA::Widget &w)
{
    if (w.m_type != HDA::WidgetType::PinComplex)
        return false;

    sendVerb(0, w.m_ID, 0xF07, 0);
    auto ctrl = readVerbResponse();

    if (!ctrl.has_value())
        return false;

    auto value = ctrl.value_or(0) & 0x07;

    sendVerb(0, w.m_ID, 0x707, value);
    ctrl = readVerbResponse();
    if (!ctrl.has_value())
        return false;

    return true;    
}

bool HDACodec::enablePinWidget(HDA::Widget &w)
{
    if (w.m_type != HDA::WidgetType::PinComplex)
        return false;

    sendVerb(0, w.m_ID, 0xF07, 0);
    auto ctrl = readVerbResponse();

    if (!ctrl.has_value())
        return false;

    auto value = ctrl.value_or(0) & 0x07;
    value |= (1<<6);

    if (w.m_headphoneDriver)
        value |= (1<<7);

    sendVerb(0, w.m_ID, 0x707, value);
    ctrl = readVerbResponse();
    if (!ctrl.has_value())
        return false;

    return true;
}

// add pin sense, page 168

bool HDACodec::setIOWidgetFormat(HDA::Widget &w, const HDA::StreamFormat &format)
{
    if ((w.m_type != HDA::WidgetType::Output) && (w.m_type != HDA::WidgetType::Input))
        return false;    

    sendVerb(0, w.m_ID, 0x200, format.m_value & 0xFFFF);
    auto ctrl = readVerbResponse();

    if (!ctrl.has_value())
        return false;

    return true;
}

std::optional<HDA::StreamFormat> HDACodec::getIOWidgetFormat(HDA::Widget &w)
{
    if ((w.m_type != HDA::WidgetType::Output) && (w.m_type != HDA::WidgetType::Input))
        return std::nullopt; 

    sendVerb(0, w.m_ID, 0xA00, 0);
    auto ctrl = readVerbResponse();

    if (!ctrl.has_value())
        return std::nullopt;

    HDA::StreamFormat format;
    format.m_value = ctrl.value_or(0);

    return format;
}

#if 0
void HDACodec::inputStreamTurnOff()
{
    write32(m_inputDescriptor, 0x00200000);
}
#endif

void HDACodec::outputStreamTurnOff()
{
    write32(m_inputDescriptor, 0x00140000);
}

#if 0
void HDACodec::inputStreamTurnOn()
{
    write32(m_inputDescriptor, 0x00140002);
}
#endif

void HDACodec::outputStreamTurnOn()
{
    write32(m_outputDescriptor, 0x00140002);
}

bool HDACodec::setOutputNode(uint32_t node)
{
    sendVerb(0, node, 0xF00, 0x12);  // get amplifier info
    auto result = readVerbResponse();
    if (!result.has_value())
        return false;

    auto ampCap = result.value_or(0);
    auto maxVolume = ampCap & 0x7F;     // actually 0 dB setting

    sendVerb(0, node, 0x706 /* set stream */, 0x10 /* stream 1 channel 1 */);
    result = readVerbResponse();
    if (!result.has_value())
        return false;
    
    sendVerb(0, node, 0x300 /* set volume */, maxVolume | 0xF000);
    result = readVerbResponse();
    if (!result.has_value())
        return false;

    return true;
}

#if 0
void HDACodec::inputStreamSetBuffer(size_t address)
{
    write32(m_inputDescriptor+0x18, address);
    write32(m_inputDescriptor+0x18+4, 0);

    write32(m_inputDescriptor+0x0C, 1); // last valid index: two entries
}
#endif

void HDACodec::outputStreamSetDescriptorList()
{
    write32(m_outputDescriptor+0x18, m_descriptorBuffer);
    write32(m_outputDescriptor+0x18+4, 0);
    write32(m_outputDescriptor+0x0C, 1); // last valid index: two entries

    uint32_t* descriptorListPtr = (uint32_t*)m_descriptorBuffer;
    descriptorListPtr[0] = m_audioBuffer;
    descriptorListPtr[1] = 0;
    descriptorListPtr[2] = 4096*16;
    descriptorListPtr[3] = 0;
    descriptorListPtr[4] = m_audioBuffer;
    descriptorListPtr[5] = 0;
    descriptorListPtr[6] = 4096*16;
    descriptorListPtr[7] = 0;
}

size_t HDACodec::getPlayPos()
{
    return read32(m_outputDescriptor+0x04);
}

void HDACodec::setSSync()
{    
}

void HDACodec::outputStreamLength(size_t length)
{
    write32(m_outputDescriptor+0x08, length);
}

void HDACodec::outputStreamFormat(const HDA::StreamFormat &format)
{
    write32(m_outputDescriptor+0x12, format.m_value);
}

void HDACodec::playSound(size_t length, const HDA::StreamFormat &format)
{
    outputStreamSetDescriptorList();
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

static uint32_t getSupportedStreams(HDACodec &codec, uint32_t widgetID)
{
    codec.sendVerb(0, widgetID, 0xF00, 0x0A);   // supported stream
    auto result = codec.readVerbResponse();
    return result.value_or(0);
}

static HDA::Amp getOutputAmplifierInfo(HDACodec &codec, uint32_t widgetID)
{
    // check output amplifier
    HDA::Amp amp;
    uint32_t settings = 0xFFFFFFFF;
    
    codec.sendVerb(0, widgetID, 0xF00, 0x12);
    auto result = codec.readVerbResponse();
    auto ampCap = result.value_or(0);
    if (ampCap != 0)
    {
        amp.m_canMute = ((ampCap >> 31) & 0x01) != 0;
        amp.m_0dBSetting = ampCap & 0x7F;
        amp.m_valid = true;
    }
    else
    {
        return amp;
    }

    codec.sendVerb(0, widgetID, 0xB00, (1<<15) /* output amp R */);
    result = codec.readVerbResponse();
    settings = result.value_or(0);
    if (result.has_value())
    {
        amp.m_muted[1]  = (((settings >> 7) & 0x01) != 0);
        amp.m_volume[1] = settings & 0x7F;
    }

    codec.sendVerb(0, widgetID, 0xB00, (1<<15) | (1<<13) /* output amp L */);
    result = codec.readVerbResponse();
    settings = result.value_or(0);
    if (result.has_value())
    {
        amp.m_muted[0]  = (((settings >> 7) & 0x01) != 0);
        amp.m_volume[0] = settings & 0x7F;
    }

    return amp;
}

static HDA::Amp getInputAmplifierInfo(HDACodec &codec, uint32_t widgetID, uint32_t channel)
{
    HDA::Amp amp;
    uint32_t settings = 0xFFFFFFFF;

    codec.sendVerb(0, widgetID, 0xF00, 0x0D);
    auto result = codec.readVerbResponse();
    auto ampCap = result.value_or(0);
    if (ampCap != 0)
    {
        amp.m_canMute = ((ampCap >> 31) & 0x01) != 0;
        amp.m_0dBSetting = ampCap & 0x7F;
        amp.m_valid = true;
    }
    else
    {
        return amp;
    }

    codec.sendVerb(0, widgetID, 0xB00, channel & 0x0F);
    result = codec.readVerbResponse();
    settings = result.value_or(0);
    if (result.has_value())
    {
        amp.m_muted[1]  = (((settings >> 7) & 0x01) != 0);
        amp.m_volume[1] = settings & 0x7F;
    }

    codec.sendVerb(0, widgetID, 0xB00, (channel & 0x0F) | (1<<13));
    result = codec.readVerbResponse();
    settings = result.value_or(0);
    if (result.has_value())
    {
        amp.m_muted[0]  = (((settings >> 7) & 0x01) != 0);
        amp.m_volume[0] = settings & 0x7F;
    }

    return amp;
}

void HDACodec::collectWidgetInformation()
{
    m_widgets.resize(64);

    for(uint32_t widgetID = 0; widgetID < 64; widgetID++)
    {
        HDA::Widget widget(widgetID);

        sendVerb(0, widgetID, 0xF00, 0x09);
        auto result = readVerbResponse();
        if (result.has_value())
        {
            auto widgetCap = result.value_or(0);

            if (widgetCap == 0)
            {
                continue;
            }
        
            if (widgetCap == 0xFFFFFFFF)
            {
                continue;
            }

            uint32_t nodeType   = (widgetCap >> 20) & 0x0F;
            uint32_t procWidget = (widgetCap >> 6) & 1;
            uint32_t connList   = (widgetCap >> 8) & 1;
            
            widget.m_digital    = ((widgetCap >> 9) & 1) != 0;
            widget.m_hasInputAmp  = ((widgetCap >> 1) & 1) != 0;
            widget.m_hasOutputAmp = ((widgetCap >> 2) & 1) != 0;

            switch(nodeType)
            {
            case 0: // Output (DAC)
                widget.m_type = HDA::WidgetType::Output;
                widget.m_supportedStreams = getSupportedStreams(*this, widgetID);
                break;
            case 1: // Input (ADC)
                widget.m_type = HDA::WidgetType::Input;
                widget.m_supportedStreams = getSupportedStreams(*this, widgetID);
                break;
            case 2: // Mixer 
                widget.m_type = HDA::WidgetType::Mixer;
                break;                
            case 3: // Selector
                widget.m_type = HDA::WidgetType::Selector;
                break;                
            case 4: // Pin complex
                widget.m_type = HDA::WidgetType::PinComplex;
                break;    
            case 7: // Beep generator                                            
                widget.m_type = HDA::WidgetType::BeepGen;
                break;                
            case 0x0F:
                widget.m_type = HDA::WidgetType::Vendor;
                break;            
            }

            widget.m_channels = 1+ ((((widgetCap >> 13) & 0x07) << 1) | (widgetCap & 1));
            
            if (widget.m_type == HDA::WidgetType::PinComplex)
            {
                sendVerb(0, widgetID, 0xF1C, 0x00);     // default config
                result = readVerbResponse();
                if (result.has_value())
                {
                    uint32_t config = result.value_or(0);
                    uint32_t connectivity = (config >> 30) & 0x3;
                    uint32_t defaultDevice = (config >> 20) & 0xF;
                    uint32_t connType = (config >> 16) & 0xF;
                    widget.m_jackInfo.m_colour = (config >> 12) & 0xF;
                    uint32_t misc = (config >> 8) & 0xF;
                    uint32_t defaultAss = (config >> 4) & 0xF;
                    uint32_t sequence = config & 0xF;
                    uint32_t jackLocation = (config >> 24) & 0xF;

                    if (connectivity != 1)
                    {
                        widget.m_jackInfo.m_isPhysical = true;
                    }

                    switch(jackLocation)
                    {
                    case 1:
                        widget.m_jackInfo.m_location = HDA::Location::Rear;
                        break;
                    case 2:
                        widget.m_jackInfo.m_location = HDA::Location::Front;
                        break;                        
                    case 3:
                        widget.m_jackInfo.m_location = HDA::Location::Left;
                        break;                                                
                    case 4:
                        widget.m_jackInfo.m_location = HDA::Location::Right;
                        break;                                                                    
                    default:
                        break;
                    }

                    switch(defaultDevice)
                    {
                    case 0: 
                        widget.m_defaultDevice = HDA::DefaultDevice::LineOut;
                        break;
                    case 1:
                        widget.m_defaultDevice = HDA::DefaultDevice::Speaker;
                        break;          
                    case 2:
                        widget.m_defaultDevice = HDA::DefaultDevice::HeadphoneOut;
                        break;
                    case 4:
                        widget.m_defaultDevice = HDA::DefaultDevice::SPDIFout;
                        break;                    
                    case 8:
                        widget.m_defaultDevice = HDA::DefaultDevice::LineIn;
                        break;                    
                    case 0xA:
                        widget.m_defaultDevice = HDA::DefaultDevice::MicIn;
                        break;
                    case 0xC:
                        widget.m_defaultDevice = HDA::DefaultDevice::SPDIFin;
                        break;                        
                    default:
                        break;
                    }
                }

                sendVerb(0, widgetID, 0xF00, 0x0C);     // pin capabilities
                result = readVerbResponse();
                if (result.has_value())
                {
                    auto pinCap = result.value_or(0);
                    auto pinTypeBits = pinCap & 0x30;
                    if (pinTypeBits & 0x10)
                    {
                        widget.m_outputPin = true;
                    }
                    if (pinTypeBits & 0x20)
                    {
                        widget.m_inputPin = true;
                    }
                    if (((pinCap >> 3) & 1) != 0)
                    {
                        widget.m_headphoneDriver = true;
                    }
                }
            }

            // connection list
            if (connList != 0)
            {
                sendVerb(0, widgetID, 0xF00, 0x0E);     // connection list length
                result = readVerbResponse();
                if (result.has_value())
                {
                    uint32_t len = result.value_or(0) & 0x7F;
                    uint32_t longForm = (result.value_or(0) >> 7) & 0x1;

                    if (longForm == 0)
                    {
                        widget.m_connectionList.resize(len);

                        uint32_t offset = 0;
                        uint32_t data = 0;
                        while(offset < len)
                        {
                            data >>= 8;
                            if ((offset % 4) == 0)
                            {
                                sendVerb(0, widgetID, 0xF02, offset);     // get connection list entry
                                result = readVerbResponse();
                                data = result.value_or(0);
                            }

                            if (result.has_value())
                            {
                                widget.m_connectionList[offset] = data & 0x7F;
                            }

                            offset++;
                        }
                    }
                    else
                    {
                        print("  long form (unsupported)");
                    }

                    if ((len != 0) && (nodeType != 2 /* mixer */))
                    {
                        sendVerb(0, widgetID, 0xF01, 0x00);
                        auto result = readVerbResponse();
                        if (result.has_value())
                        {
                            widget.m_selection = result.value_or(0) & 0xFF;
                        }
                    }
                }
            }

            if (widget.m_hasInputAmp)
            {
                // input and output amplifiers
                uint32_t numberOfAmps = 1;
                if (widget.m_type == HDA::WidgetType::Mixer)
                    numberOfAmps = widget.m_connectionList.size();

                widget.m_inputAmps.resize(numberOfAmps);
                size_t ch = 0;
                for(auto &amp : widget.m_inputAmps)
                {
                    amp = getInputAmplifierInfo(*this, widgetID, ch);
                    ch++;
                };
            }

            if (widget.m_hasOutputAmp)
            {
                widget.m_outputAmp = getOutputAmplifierInfo(*this, widgetID);
            }
        }
        
        m_widgets[widgetID] = widget;
    }
}

static void dumpAmp(const HDA::Amp &amp)
{
    std::array<const char *, 2> side = {{"left", "right"}};

    if (!amp.m_valid)
        return;

    for(uint32_t t=0; t<2; t++)
    {
        print("  %s: %d", side[t], amp.m_volume[t]);
        if (amp.m_canMute)
        {
            print(" state: %s      ", amp.m_muted[t] ? "MUTED" : "unmuted");
        }
    }
    print("\n\r");
}

static void dumpSupportedStreams(const uint32_t supportedStreams)
{
    if ((supportedStreams & 1) != 0)
    {
        print("  8000 sps\n\r");
    }
    if ((supportedStreams & 2) != 0)
    {
        print("  11025 sps\n\r");
    }
    if ((supportedStreams & 4) != 0)
    {
        print("  16000 sps\n\r");
    }
    if ((supportedStreams & 8) != 0)
    {
        print("  22050 sps\n\r");
    }                   
    if ((supportedStreams & 16) != 0)
    {
        print("  32000 sps\n\r");
    }
    if ((supportedStreams & 32) != 0)
    {
        print("  44100 sps\n\r");
    }                    
    if ((supportedStreams & 64) != 0)
    {
        print("  48000 sps\n\r");
    }
    if ((supportedStreams & 128) != 0)
    {
        print("  88200 sps\n\r");
    }
    if ((supportedStreams & 256) != 0)
    {
        print("  96000 sps\n\r");
    }     
    if ((supportedStreams & 512) != 0)
    {
        print("  1764000 sps\n\r");
    }                    
    if ((supportedStreams & 1024) != 0)
    {
        print("  1920000 sps\n\r");
    }                         
    if ((supportedStreams & 2048) != 0)
    {
        print("  3840000 sps\n\r");
    }      

    if (((supportedStreams >> 16) & 1) != 0)
    {
        print("  8-bit stream\n\r");
    }
    if (((supportedStreams >> 17) & 1) != 0)
    {
        print("  16-bit stream\n\r");
    }   
    if (((supportedStreams >> 18) & 1) != 0)
    {
        print("  20-bit stream\n\r");
    }      
    if (((supportedStreams >> 19) & 1) != 0)
    {
        print("  24-bit stream\n\r");
    }
    if (((supportedStreams >> 20) & 1) != 0)
    {
        print("  32-bit stream\n\r");
    }        
}

static void dumpInput(const HDA::Widget &w)
{
    if (w.m_hasInputAmp)
    {
        print("  input amplifiers:\n\r");
        for(uint32_t i=0; i<w.m_inputAmps.size(); i++)
        {
            print("  ch %d ", i);
            dumpAmp(w.m_inputAmps[i]);
        }
    }

    if (w.m_hasOutputAmp)
    {
        print("  output amplifier:\n\r");
        dumpAmp(w.m_outputAmp);
    }

    if (w.m_connectionList.size() > 0)
    {
        print("  connected to nodes: ");
        size_t oID = 0;
        for(auto o : w.m_connectionList)
        {
            print("%d ", o);
            oID++;
        }
        print("\n\r");
    }

    dumpSupportedStreams(w.m_supportedStreams);
}

static void dumpOutput(const HDA::Widget &w)
{
    if (w.m_hasInputAmp)
    {
        print("  input amplifiers:\n\r");
        for(uint32_t i=0; i<w.m_inputAmps.size(); i++)
        {
            print("  ch %d ", i);
            dumpAmp(w.m_inputAmps[i]);
        }
    }

    if (w.m_hasOutputAmp)
    {
        print("  output amplifier: ");
        dumpAmp(w.m_outputAmp);
    }

    if (w.m_connectionList.size() > 0)
    {
        print("  connected to nodes: ");
        size_t oID = 0;
        for(auto o : w.m_connectionList)
        {
            print("%d ", o);
            oID++;
        }
        print("\n\r");
    }

    dumpSupportedStreams(w.m_supportedStreams);
}

static void dumpMixer(const HDA::Widget &w)
{
    print("  input come from nodes: ");
    size_t oID = 0;
    for(auto o : w.m_connectionList)
    {
        print("%d ", o);
        oID++;
    }
    print("\n\r");

    if (w.m_hasInputAmp)
    {
        print("  input amplifiers:\n\r");
        for(uint32_t i=0; i<w.m_inputAmps.size(); i++)
        {
            print("  ch %d ", i);
            dumpAmp(w.m_inputAmps[i]);
        }
    }

    if (w.m_hasOutputAmp)
    {
        print("  output amplifier: ");
        dumpAmp(w.m_outputAmp);
    }
}

static void dumpJackInfo(const HDA::JackInfo &info)
{
    print("  Jack colour: %s\n\r", HDA::JackInfo::colours[info.m_colour & 0xF]);
    switch(info.m_location)
    {
    case HDA::Location::Front:
        print("    Front\n\r");
        break;
    case HDA::Location::Rear:
        print("    Rear\n\r");
        break;        
    case HDA::Location::Left:
        print("    Left\n\r");
        break;                
    case HDA::Location::Right:
        print("    Right\n\r");
        break;          
    default:
        print("    Unknown location\n\r");
        break;                                
    }
}

static void dumpPinComplex(const HDA::Widget &w)
{
    print("  Default function: ");
    switch(w.m_defaultDevice)
    {
    case HDA::DefaultDevice::LineIn:
        print("Line in\n\r");
        break;
    case HDA::DefaultDevice::LineOut:
        print("Line out\n\r");
        break;        
    case HDA::DefaultDevice::MicIn:
        print("Mic in\n\r");
        break;                
    case HDA::DefaultDevice::Speaker:
        print("Speaker\n\r");
        break;
    case HDA::DefaultDevice::HeadphoneOut:
        print("Headphone\n\r");
        break;        
    case HDA::DefaultDevice::SPDIFin:
        print("SPDIF in\n\r");
        break;         
    case HDA::DefaultDevice::SPDIFout:
        print("SPDIF out\n\r");
        break;                        
    default:
        print("Unknown\n\r");
        break;
    }

    if (w.m_headphoneDriver)
    {
        print("  Can drive a headphone\n\r");
    }
    if (w.m_inputPin)
    {
        print("  Is an input pin\n\r");
    }    
    if (w.m_outputPin)
    {
        print("  Is an output pin\n\r");
    }
    
    dumpJackInfo(w.m_jackInfo); 

    if (!w.m_connectionList.empty())
    {
        print("  connected to nodes: ");
        size_t oID = 0;
        for(auto o : w.m_connectionList)
        {
            if (oID == w.m_selection)
            {
                print("%d (selected)    ", o);
            }
            else
            {
                print("%d ", o);
            }
            oID++;
        }
        print("\n\r");
    }    
}


void HDA::Widget::dump()
{
    print("ID = %d\n\r", m_ID);
    if (m_digital)
        print("  digital device\n\r");

    switch(m_type)
    {
    case HDA::WidgetType::Input:
        print("  Input\n\r");
        dumpInput(*this);
        break;
    case HDA::WidgetType::Output:
        print("  Output\n\r");
        dumpOutput(*this);
        break;
    case HDA::WidgetType::Mixer:
        print("  Mixer\n\r");
        dumpMixer(*this);
        break;
    case HDA::WidgetType::Selector:
        print("  Selector\n\r");
        break; 
    case HDA::WidgetType::PinComplex:
        print("  Pin Complex\n\r");
        dumpPinComplex(*this);
        break;
    case HDA::WidgetType::BeepGen:
        print("  Beep generator\n\r");
        break; 
    case HDA::WidgetType::Vendor:
        print("  Vendor defined\n\r");
        break;        
    default:
        print("  Unknown\n\r");
        return;
    }
}


void refreshWidget(HDACodec &codec, HDA::Widget &w)
{
    // refresh amp settings
    if (!w.m_inputAmps.empty())
    {
        size_t channel = 0;
        for(auto& amp: w.m_inputAmps)
        {
            amp = getInputAmplifierInfo(codec, w.m_ID, channel);
            channel++;
        }
    }

    if (w.m_outputAmp.m_valid)
    {
        w.m_outputAmp = getOutputAmplifierInfo(codec, w.m_ID);
    }
}


bool HDA::StreamFormat::set(SampleRate sr, BitDepth bits, uint32_t channels)
{
    if (sr == SampleRate::Invalid) return false;
    if (bits == BitDepth::Invalid) return false;
    if (channels > 16) return false;
    if (channels == 0) return false;

    m_value  = channels-1;
    switch(bits)
    {
    case BitDepth::B8:
        break;
    case BitDepth::B16:
        m_value |= (1<<4);
        break;
    case BitDepth::B20:
        m_value |= (2<<4);
        break;
    case BitDepth::B24:
        m_value |= (3<<4);
        break; 
    case BitDepth::B32:
        m_value |= (4<<4);
        break;
    default:
        return false;
    }
    
    uint32_t baseRate = 0;
    uint32_t multiplier = 0;
    uint32_t divisor = 0;
    switch(sr)
    {
    case SampleRate::R16000:
        baseRate   = 0; // 48 kHz
        multiplier = 0; // 48kHz or less
        divisor    = 2; // divide by 3
        break;
    case SampleRate::R22050:
        baseRate   = 1; // 44.1 kHz
        multiplier = 0; // 48kHz or less
        divisor    = 1; // divide by 2
        break;
    case SampleRate::R32000:
        baseRate   = 0; // 48 kHz
        multiplier = 1; // x2 
        divisor    = 2; // divide by 3
        break;  
    case SampleRate::R44100:
        baseRate   = 1; // 44.1 kHz
        multiplier = 0; // 48kHz or less
        divisor    = 0; // no-divide
        break;      
    case SampleRate::R48000:
        baseRate   = 0; // 48 kHz
        multiplier = 0; // 48kHz or less
        divisor    = 0; // no-divide
        break;              
    case SampleRate::R96000:
        baseRate   = 0; // 48 kHz
        multiplier = 1; // x2
        divisor    = 0; // no-divide
        break;                      
    case SampleRate::R192000:
        baseRate   = 0; // 48 kHz
        multiplier = 3; // x4
        divisor    = 0; // no-divide
        break;
    };

    m_value |= (baseRate << 14);
    m_value |= (multiplier << 11);
    m_value |= (divisor << 8);

    return true;
}

HDA::SampleRate HDA::StreamFormat::getSampleRate() const
{
    uint32_t baseRate   = (m_value >> 14) & 0x1;
    uint32_t multiplier = (m_value >> 11) & 0x7;
    uint32_t divisor    = (m_value >> 8) & 0x7;

    uint32_t rate = (baseRate == 0) ? 48000:44100;
    rate *= (multiplier+1);
    rate /= (divisor+1);

    switch(rate)
    {
    case 16000:
        return HDA::SampleRate::R16000;
    case 32000:
        return HDA::SampleRate::R32000;
    case 44100:
        return HDA::SampleRate::R44100;
    case 48000:
        return HDA::SampleRate::R48000;
    case 96000:
        return HDA::SampleRate::R96000;
    case 192000:
        return HDA::SampleRate::R192000;
    default:
        return HDA::SampleRate::Invalid;
    }
};

HDA::BitDepth HDA::StreamFormat::getBitDepth() const
{
    uint32_t v = (m_value >> 4) & 0x7;
    switch (v)
    {
    case 0:
        return HDA::BitDepth::B8;
    case 1:
        return HDA::BitDepth::B16;
    case 2:
        return HDA::BitDepth::B20;
    case 3:
        return HDA::BitDepth::B24;
    case 4:
        return HDA::BitDepth::B32;
    default:
        return HDA::BitDepth::Invalid;
    }
}

uint32_t HDA::StreamFormat::getChannels() const
{
    return (m_value & 0xF) + 1;
}

const char* HDA::toString(const HDA::BitDepth &d)
{
    constexpr std::array<const char *, 6> strings =
    {{
        "Invalid", "8 bits", "16 bits", "20 bits", "24 bits", "32 bits"
    }};

    auto dd = static_cast<uint8_t>(d);

    if ((dd < 0) || (dd >= strings.size()))
        return strings[0];
    
    return strings[dd];
}

const char* HDA::toString(const HDA::SampleRate &sr)
{
    constexpr std::array<const char *, 8> strings =
    {{
        "Invalid", "16000", "22050", "32000", "44100", "48000", "96000", "192000"
    }};

    auto dd = static_cast<uint8_t>(sr);

    if ((dd < 0) || (dd >= strings.size()))
        return strings[0];
    
    return strings[dd];
}

