#include <array>
#include <optional>
#include "commands.h"


extern size_t print(const char *fmt, ...);

extern "C"
{
    int memcmp(const char *s1, const char *s2, size_t count)
    {
        while (count-- > 0)
        {
            if (*s1++ != *s2++)
                return s1[-1] < s2[-1] ? -1 : 1;
        }
        return 0;    
    }
}


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
            value << 4;
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
        print("Invalid value %s\n\r", regvalue.m_param);
    }

    return false;
}

struct commandDef
{
    const char *name;
    commandPrototype m_func;
};

static constexpr const std::array<commandDef, 2> g_commands {
    {{"read", cmdReadCodecRegister},
     {"write", cmdWriteCodecRegister}
     //{"read", cmdTest}}
    }
};

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

