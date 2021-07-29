#include <array>
#include <optional>
#include "commands.h"


extern size_t print(const char *fmt, ...);

constexpr bool isAlpha(char c)
{
    if ((c>='a') && (c<='z')) return true;
    if ((c>='A') && (c<='Z')) return true;
    return false;
}

constexpr bool isNumeric(char c)
{
    if ((c>='0') && (c<='9')) return true;
    return false;
}

constexpr bool isHex(char c)
{
    if ((c>='0') && (c<='9')) return true;
    if ((c>='a') && (c<='f')) return true;
    if ((c>='A') && (c<='F')) return true;
    return false;
}

static constexpr uint32_t hexchar2value(char c)
{
    if ((c >= '0') && (c <= '9'))
        return (c-'0') & 0x0F;
    if ((c >= 'a') && (c <= 'f'))
        return (c-'a'+10) & 0x0F;
    if ((c >= 'A') && (c <= 'F'))
        return (c-'A'+10) & 0x0F;
    
    return 0;
}

std::optional<uint64_t> valueFromHexStr(const std::string_view &str)
{
    uint64_t value = 0;
    
    // note, first char is '$' so we skip that!
    for(uint32_t i=1; i<str.size(); i++)
    {
        char c = str[i];
        if (isHex(c))
        {
            value <<= 4;
            value |= hexchar2value(c);
        }   
        else
        {
            // error! not an integer
            return std::nullopt;
        }
    }    
    return value;
}

std::optional<uint64_t> valueFromDecStr(const std::string_view &str)
{
    uint64_t value = 0;
    for(uint32_t i=0; i<str.size(); i++)
    {
        char c = str[i];
        if (isNumeric(c))
        {
            value *= 10;
            value += c - '0';
        }   
        else
        {
            // error! not an integer
            return std::nullopt;
        }
    }    
    return value;
}

std::optional<uint64_t> valueFromStr(const std::string_view &str)
{
    if (str.size() == 0)
        return std::nullopt;

    // hex always starts with '$'
    if (str[0] == '$')
    {
        return valueFromHexStr(str);
    }
    else
    {
        return valueFromDecStr(str);
    }
}

struct paramResult
{
    std::string_view m_param;
    std::string_view m_remainder;
};

paramResult findNextParam(const std::string_view &params)
{
    size_t start = 0;
    size_t end   = 0;
    while(params[start] == ' ') 
    {
        start++;
        if (start == params.size())
            return paramResult();   // no parameters left
    }
    
    end = start;
    while((params[end] != ' ') && (end != params.size()))
    {
        end++;
    }

    return paramResult{std::string_view(params.data()+start, end-start), std::string_view(params.data()+end, params.size() - end)};
}

static bool cmdTest(HDACodec &codec, const std::string_view &params)
{
    print("TEST! ");

    size_t paramcount = 0;
    auto r = findNextParam(params);
    while(r.m_param.size() > 0)
    {
        paramcount++;
        print("'");
        for(size_t i=0; i<r.m_param.size(); i++)
            print("%c", r.m_param[i]);
        print("' ");

        r = findNextParam(r.m_remainder);
    };

    print("\n\r  param count = %d\n\r", paramcount);

    return true;
}

#if 0
static void showAmplifierSettings(HDACodec &codec, uint32_t node, uint32_t chanCount)
{
    uint32_t count = 0;
    uint32_t settings = 0xFFFFFFFF;

    while(count < chanCount)
    {
        codec.sendVerb(0, node, 0xB00, count & 0x0F);
        auto result = codec.readVerbResponse();
        settings = result.value_or(0);
        if (settings != 0)
        {
            int32_t mute = (settings >> 7) & 1;
            int32_t gain = settings & 0x7F;
            print("    input %d R : %d %s\n\r", count, gain, (mute == 1) ? "MUTED" : "");
        }

        codec.sendVerb(0, node, 0xB00, (count & 0x0F) | (1<<13));
        result = codec.readVerbResponse();
        settings = result.value_or(0);
        if (settings != 0)
        {
            int32_t mute = (settings >> 7) & 1;
            int32_t gain = settings & 0x7F;
            print("    input %d L : %d %s\n\r", count, gain, (mute == 1) ? "MUTED" : "");
        }

        count++;
    }

    // check output amplifier
    codec.sendVerb(0, node, 0xB00, (1<<15) /* output amp R */);
    auto result = codec.readVerbResponse();
    settings = result.value_or(0);
    if (settings != 0)
    {
        int32_t mute = (settings >> 7) & 1;
        int32_t gain = settings & 0x7F;
        print("    output R : %d %s\n\r", gain, (mute == 1) ? "MUTED" : "");
    }

    codec.sendVerb(0, node, 0xB00, (1<<15) | (1<<13) /* output amp L */);
    result = codec.readVerbResponse();
    settings = result.value_or(0);
    if (settings != 0)
    {
        int32_t mute = (settings >> 7) & 1;
        int32_t gain = settings & 0x7F;
        print("    output L : %d %s\n\r", gain, (mute == 1) ? "MUTED" : "");
    }
}

static void showSelection(HDACodec &codec, uint32_t nodeNum)
{
    codec.sendVerb(0, nodeNum, 0xF01, 0x00);
    auto result = codec.readVerbResponse();
    if (result.has_value())
    {
        int32_t index = result.value_or(0) & 0xFF;
        print("  Selector: index=%d\n\r", index);
    }
}
#endif

static bool cmdUnmute(HDACodec &codec, const std::string_view &params)
{
    auto widgetParam = findNextParam(params);
    auto widgetID = valueFromStr(widgetParam.m_param).value_or(0);

    auto channelParam = findNextParam(widgetParam.m_remainder);
    auto channel = valueFromStr(channelParam.m_param).value_or(0);

    auto gainSetting = 0x7F;
    if (codec.m_widgets[widgetID].m_outputAmp.m_valid)
    {
        gainSetting = codec.m_widgets[widgetID].m_outputAmp.m_0dBSetting;
    }
    else if (codec.m_widgets[widgetID].m_inputAmps.size() > 0)
    {
        gainSetting = codec.m_widgets[widgetID].m_inputAmps[0].m_0dBSetting;
    }

    print("Unmuting channel %d on node %d (gain = %d)\n\r", channel, widgetID, gainSetting);

    codec.sendVerb(0, widgetID, 0x0300, 0x7000 | ((channel & 0xF) << 8) | gainSetting);
    auto dummy = codec.readVerbResponse();

    codec.sendVerb(0, widgetID, 0x0300, 0xB000 | gainSetting);
    dummy = codec.readVerbResponse();

    return true;
}

static bool cmdRefresh(HDACodec &codec, const std::string_view &params)
{
    for(auto& widget : codec.m_widgets)
    {
        refreshWidget(codec, widget);
    };

    return true;
}

static bool cmdNodeIO(HDACodec &codec, const std::string_view &params)
{
    auto widgetParam = findNextParam(params);
    auto widgetID = valueFromStr(widgetParam.m_param).value_or(0);

    auto verbParam = findNextParam(widgetParam.m_remainder);
    auto verb = valueFromStr(verbParam.m_param).value_or(0);
  
    auto payloadParam = findNextParam(verbParam.m_remainder);
    auto payload = valueFromStr(payloadParam.m_param).value_or(0);

    print("NodeIO: %d verb: %x  payload %x\n\r", widgetID, verb, payload);
    codec.sendVerb(0, widgetID, verb, payload);

    auto result = codec.readVerbResponse();
    if (result.has_value())
    {
        print("  retuned: %x\n\r", result.value_or(0));
    }
    else
    {
        print("  ** no response! **\n\r");
    }
    return true;
}

static bool cmdDisplayNode(HDACodec &codec, const std::string_view &params)
{

    auto widgetParam = findNextParam(params);
    auto widgetID = valueFromStr(widgetParam.m_param);

    if (widgetID.has_value())
    {
        auto id = widgetID.value_or(0);

        if (id < codec.m_widgets.size())
        {
            codec.m_widgets[id].dump();
            return true;
        }
        else
        {
            print("ERROR: ID out of range\n\r");
            return false;
        }
    }

    print("Parameter error\n\r");
    return false;

#if 0

    if (nodeNum.has_value())
    {
        codec.sendVerb(0, nodeNum.value_or(0), 0xF00, 0x09);
        auto result = codec.readVerbResponse();
        if (result.has_value())
        {
            auto node = nodeNum.value_or(0);
            auto widgetCap = result.value_or(0);

            if (widgetCap == 0)
            {
                print("No information for node %d\n\r", node);
                return false;
            }
            
            if (widgetCap == 0xFFFFFFFF)
            {
                print("No information for node %d\n\r", node);
                return false;
            }

            uint32_t nodeType   = (widgetCap >> 20) & 0x0F;
            uint32_t procWidget = (widgetCap >> 6) & 1;
            uint32_t connList   = (widgetCap >> 8) & 1;
            uint32_t digital    = (widgetCap >> 9) & 1;

            // chanCount: see 7.3.4.6 of HDAudio spec
            uint32_t chanCount = 1+ ((((widgetCap >> 13) & 0x07) << 1) | (widgetCap & 1));
            switch(nodeType)
            {
            case 0: 
                print("Audio Output: %d - channels: %d\n\r", node, chanCount);
                codec.sendVerb(0, node, 0xF00, 0x0A);   // supported stream
                result = codec.readVerbResponse();
                if (result.has_value())
                {
                    auto stream = result.value_or(0);
                    if ((stream & 1) != 0)
                    {
                        print("  8000 sps\n\r");
                    }
                    if ((stream & 2) != 0)
                    {
                        print("  11025 sps\n\r");
                    }
                    if ((stream & 4) != 0)
                    {
                        print("  16000 sps\n\r");
                    }
                    if ((stream & 8) != 0)
                    {
                        print("  22050 sps\n\r");
                    }                   
                    if ((stream & 16) != 0)
                    {
                        print("  32000 sps\n\r");
                    }
                    if ((stream & 32) != 0)
                    {
                        print("  44100 sps\n\r");
                    }                    
                    if ((stream & 64) != 0)
                    {
                        print("  48000 sps\n\r");
                    }
                    if ((stream & 128) != 0)
                    {
                        print("  88200 sps\n\r");
                    }
                    if ((stream & 256) != 0)
                    {
                        print("  96000 sps\n\r");
                    }     
                    if ((stream & 512) != 0)
                    {
                        print("  1764000 sps\n\r");
                    }                    
                    if ((stream & 1024) != 0)
                    {
                        print("  1920000 sps\n\r");
                    }                         
                    if ((stream & 2048) != 0)
                    {
                        print("  3840000 sps\n\r");
                    }      

                    if (((stream >> 16) & 1) != 0)
                    {
                        print("  8-bit stream\n\r");
                    }
                    if (((stream >> 17) & 1) != 0)
                    {
                        print("  16-bit stream\n\r");
                    }   
                    if (((stream >> 18) & 1) != 0)
                    {
                        print("  20-bit stream\n\r");
                    }      
                    if (((stream >> 19) & 1) != 0)
                    {
                        print("  24-bit stream\n\r");
                    }
                    if (((stream >> 20) & 1) != 0)
                    {
                        print("  32-bit stream\n\r");
                    }
                }
                else
                {
                    print("  Stream info failed\n\r");
                }
                showAmplifierSettings(codec, node, chanCount);
                break;
            case 1:
                print("Audio Input: %d - channels: %d\n\r", node, chanCount);
                codec.sendVerb(0, node, 0xF00, 0x0A);   // supported stream
                result = codec.readVerbResponse();
                if (result.has_value())
                {
                    auto stream = result.value_or(0);
                    if ((stream & 1) != 0)
                    {
                        print("  8000 sps\n\r");
                    }
                    if ((stream & 2) != 0)
                    {
                        print("  11025 sps\n\r");
                    }
                    if ((stream & 4) != 0)
                    {
                        print("  16000 sps\n\r");
                    }
                    if ((stream & 8) != 0)
                    {
                        print("  22050 sps\n\r");
                    }                   
                    if ((stream & 16) != 0)
                    {
                        print("  32000 sps\n\r");
                    }
                    if ((stream & 32) != 0)
                    {
                        print("  44100 sps\n\r");
                    }                    
                    if ((stream & 64) != 0)
                    {
                        print("  48000 sps\n\r");
                    }
                    if ((stream & 128) != 0)
                    {
                        print("  88200 sps\n\r");
                    }
                    if ((stream & 256) != 0)
                    {
                        print("  96000 sps\n\r");
                    }     
                    if ((stream & 512) != 0)
                    {
                        print("  1764000 sps\n\r");
                    }                    
                    if ((stream & 1024) != 0)
                    {
                        print("  1920000 sps\n\r");
                    }                         
                    if ((stream & 2048) != 0)
                    {
                        print("  3840000 sps\n\r");
                    }      

                    if (((stream >> 16) & 1) != 0)
                    {
                        print("  8-bit stream\n\r");
                    }
                    if (((stream >> 17) & 1) != 0)
                    {
                        print("  16-bit stream\n\r");
                    }   
                    if (((stream >> 18) & 1) != 0)
                    {
                        print("  20-bit stream\n\r");
                    }      
                    if (((stream >> 19) & 1) != 0)
                    {
                        print("  24-bit stream\n\r");
                    }
                    if (((stream >> 20) & 1) != 0)
                    {
                        print("  32-bit stream\n\r");
                    }
                }
                else
                {
                    print("  Stream info failed\n\r");
                }                                
                break;
            case 2:
                print("Audio Mixer: %d (%d channels)\n\r", node, chanCount);
                showAmplifierSettings(codec, node, chanCount);
                break;
            case 3:
                print("Audio Selector: %d\n\r", node);
                break;                
            case 4:
                print("Pin complex: %d (%d channels)\n\r", node, chanCount);
                codec.sendVerb(0, node, 0xF00, 0x0C);     // pin capabilities
                result = codec.readVerbResponse();
                if (result.has_value())
                {
                    auto pinCap = result.value_or(0);
                    auto pinTypeBits = pinCap & 0x30;
                    if (pinTypeBits & 0x10)
                        print("  output pin\n\r");
                    if (pinTypeBits & 0x20)
                        print("  input pin\n\r");
                    if (((pinCap >> 3) & 1) != 0)
                        print("  headphone capable\n\r");                    
                }

                // TODO: configuration default
                //       7.3.3.31
                codec.sendVerb(0, node, 0xF1C, 0x00);     // default config
                result = codec.readVerbResponse();
                if (result.has_value())
                {
                    uint32_t config = result.value_or(0);
                    uint32_t connectivity = (config >> 30) & 0x3;
                    uint32_t defaultDevice = (config >> 20) & 0xF;
                    uint32_t connType = (config >> 16) & 0xF;
                    uint32_t colour = (config >> 12) & 0xF;
                    uint32_t misc = (config >> 8) & 0xF;
                    uint32_t defaultAss = (config >> 4) & 0xF;
                    uint32_t sequence = config & 0xF;

                    switch(defaultDevice)
                    {
                    case 0: 
                        print("  Line Out\n\r");
                        break;
                    case 1:
                        print("  Speaker\n\r");
                        break;          
                    case 2:
                        print("  HP Out\n\r");
                        break;
                    case 8:
                        print("  Line In\n\r");
                        break;                               
                    case 0xA:
                        print("  Mic In\n\r");
                        break;
                    default:
                        break;
                    }

                    static constexpr std::array<const char *, 16> colours =
                    {{
                        "Unknown", "Black", "Grey", "Blue", "Green", "Red",
                        "Orange", "Yellow", "Purple", "Pink", "Reserved",
                        "White", "Other"
                    }};

                    print("  Colour: %s\n\r", colours[colour]);
                    print("  Default association: %d\n\r", defaultAss);
                    showAmplifierSettings(codec, node, chanCount);
                }
                break;
                case 5:
                    print("Power widget: %d\n\r", node);
                    break;
                case 6:
                    print("Volume knob: %d\n\r", node);
                    break;
                case 0xF:
                    print("Vendor defined: %d\n\r", node);
                    break;
                default:
                    print("Undefined (id=%d decimal): %d\n\r", nodeType, node);
                    break;
            }

            if (procWidget != 0)
            {
                print("  procWidget = 1\n\r");
            }

            if (digital != 0)
            {
                print("  digital = 1\n\r");
            }

            if (connList != 0)
            {
                print("  connList = 1 ");
                codec.sendVerb(0, node, 0xF00, 0x0E);     // connection list length
                result = codec.readVerbResponse();
                if (result.has_value())
                {
                    uint32_t len = result.value_or(0) & 0x7F;
                    uint32_t longForm = (result.value_or(0) >> 7) & 0x1;
                    print(" length = %d", len);

                    if (longForm == 0)
                    {
                        print("    ");
                        uint32_t offset = 0;
                        uint32_t data = 0;
                        while(offset < len)
                        {
                            data >>= 8;
                            if ((offset % 4) == 0)
                            {
                                codec.sendVerb(0, node, 0xF02, offset);     // get connection list entry
                                result = codec.readVerbResponse();
                                data = result.value_or(0);
                            }

                            if (result.has_value())
                            {
                                print("%d ", data & 0x7F);
                            }

                            offset++;
                        }
                        print("\n\r");
                    }
                    else
                    {
                        print("  long form (unsupported)");
                    }

                    if ((len != 0) && (nodeType != 2 /* mixer */))
                    {
                        showSelection(codec, node);
                    }
                }
                print("\n\r");
            }

            // subnodes
            codec.sendVerb(0, node, 0xF00, 0x04);     // subnode count
            result = codec.readVerbResponse();
            if (result.has_value())
            {
                uint32_t startNode = (result.value_or(0) >> 16) & 0xF;
                uint32_t nodeCount = result.value_or(0) & 0xF;
                if (nodeCount != 0)
                {
                    print("  subnodes: %d to %d\n\r", startNode, startNode + nodeCount-1);
                }
            }

            return true;
        }
    }

    print("Invalid nodeId\n\r");
    return false;
    #endif
}

static bool cmdPlayPos(HDACodec &codec, const std::string_view &params)
{
    print("  $%X\n\r", codec.getPlayPos());
    return true;
}

static bool cmdGetFormat(HDACodec &codec, const std::string_view &params)
{
    auto widgetParam = findNextParam(params);
    auto widgetID = valueFromStr(widgetParam.m_param);

    if (widgetID.has_value())
    {
        auto id = widgetID.value_or(0);
        if (id < codec.m_widgets.size())
        {
            auto format = codec.getIOWidgetFormat(codec.m_widgets[id]);
            if (!format.has_value())
            {
                print("Cannot get format \n\r");
                return false;
            }
            
            auto f = format.value_or(0);

            print("  format: sr=%s bits=%s channels=%d\n\r", 
                HDA::toString(f.getSampleRate()),
                HDA::toString(f.getBitDepth()),
                f.getChannels());
            
            return true;
        }
        else
        {
            print("Node ID out of range\n\r");
            return false;
        }
    }

    return false;
}

static bool cmdReadCodecRegister(HDACodec &codec, const std::string_view &params)
{
    // read regname
    auto regname = findNextParam(params);

    auto result = codec.readReg(regname.m_param);
    print("%s = $%x\n\r", regname.m_param.data(), result);
    return true;
}

static bool cmdWriteCodecRegister(HDACodec &codec, const std::string_view &params)
{
    // read regname
    auto regname = findNextParam(params);

    // read hex value
    auto regvalue = findNextParam(regname.m_remainder);
    auto value = valueFromStr(regvalue.m_param);

    if (value.has_value())
    {
        print("%s <= $%x\n\r", regname.m_param.data(), value.value_or(0));
        return codec.writeReg(regname.m_param, value.value_or(0));
    }
    else
    {
        print("Invalid value\n\r");
    }

    return false;
}

static bool cmdShowVideo(HDACodec &codec, const std::string_view &params)
{
    print("Video modes:\n\r");
    // try to find the Graphics driver
    UEFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

    if (g_sysTbl->m_bootServices->m_locateProtocol(&gopGuid, nullptr, (void**)&gop) != 0)
    {
        print("Cannot access UEFI graphics protocol\n\r");
        return false;
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

        int32_t desiredMode = -1;
        size_t  offset = 0;
        for(size_t modeIdx = 0; modeIdx < maxMode; modeIdx++)
        {
            if (modeIdx % 2 == 0)
            {
                print("\n\r");
                offset = 2;
            }
            else
            {
                while(offset < 40)
                {
                    print(" ");
                    offset++;
                }
            }

            gop->QueryMode(gop, modeIdx, &sizeOfInfo, &info);

            auto modeWidth  = info->m_horizontalResolution;
            auto modeHeight = info->m_verticalResolution;

            if (modeIdx < 10)
            {
                offset += print("mode  %d -> x=%d  y=%d        ", 
                    modeIdx,
                    modeWidth,
                    modeHeight);
            }
            else
            {
                offset += print("mode %d -> x=%d  y=%d        ", 
                    modeIdx,
                    modeWidth,
                    modeHeight);                
            }
        }
        print("\n\r");
    }    
    return true;
}

static bool cmdShowText(HDACodec &codec, const std::string_view &params)
{
    print("Text modes:\n\r");
    
    uint64_t mode = 0;
    uint64_t cols = 0;
    uint64_t rows = 0;

    auto status = g_sysTbl->m_conOut->m_queryMode(g_sysTbl->m_conOut, mode, &cols, &rows);
    while(status == EFI_SUCCESS)
    {
        print("  mode: %d   rows=%d  cols=%d\n\r", mode, cols, rows);
        mode++;
        status = g_sysTbl->m_conOut->m_queryMode(g_sysTbl->m_conOut, mode, &cols, &rows);        
    }    
    return true;
}

static bool cmdSetTextMode(HDACodec &codec, const std::string_view &params)
{
    // read mode id
    auto modeParam = findNextParam(params);
    auto mode = valueFromStr(modeParam.m_param);

    if (mode.has_value())
    {
        auto modeID = mode.value_or(0);
        auto status = g_sysTbl->m_conOut->m_setMode(g_sysTbl->m_conOut, modeID);
        if (status == EFI_SUCCESS)
        {
            return true;
        }
    }

    return false;
}

static bool cmdHelp(HDACodec &codec, const std::string_view &params);   // pre-declaration

struct commandDef
{
    const char *name;
    commandPrototype m_func;
    const char *description;
};

static constexpr const std::array<commandDef, 12> g_commands {
    {{"read", cmdReadCodecRegister, "Read codec register (reg name)"},
     {"write", cmdWriteCodecRegister, "Write codec register (reg name, data)"},
     {"node", cmdDisplayNode, "Display codec node information (nodeId)"},
     {"unmute", cmdUnmute, "Unmute a node (nodeId)"},
     {"refresh", cmdRefresh, "Update internal node database"},
     {"nodeio", cmdNodeIO, "Write a verb/command to a node (nodeId, verb, payload)"},
     {"getformat", cmdGetFormat, "Get the stream format for input/output nodes (nodeId)"},
     {"playpos", cmdPlayPos, "Get the current play position"},
     {"showvideo", cmdShowVideo, "Show available video modes"},
     {"showtext", cmdShowText, "Show available text modes"},
     {"settextmode", cmdSetTextMode, "Set text mode by id (mode id)"},
     {"help", cmdHelp, "Show this list"}
    }
};

static bool cmdHelp(HDACodec &codec, const std::string_view &params)
{
    print("Available commands:\n\r");
    for(auto cmd: g_commands)
    {
        size_t chars = print("  %s :", cmd.name);
        while( chars < 16)
        {
            print(" ");
            chars++;
        }
        print("%s\n\r", cmd.description);
    }
    return true;
}

bool Commands::execute(const std::string_view &line)
{
    auto cmd = getCommand(line);

    for(auto c : g_commands)
    {
        if (c.name == cmd)
        {
            return c.m_func(m_codec, std::string_view(line.data()+cmd.size(), line.size()-cmd.size()));
        }
    }

    return false;
}

std::string_view Commands::getCommand(const std::string_view &line) const
{
    size_t last_idx=0;
    while(isAlpha(line[last_idx]) && (last_idx < line.size()))
    {
        last_idx++;
    }

    return std::string_view(line.data(), last_idx);
}

