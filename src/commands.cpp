#include <array>
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

    if (regname.m_param == "gctl")
    {
        print("GCTL = $%x\n\r", codec.read32(HDACodec::GCTL32));
        return true;
    }

    if (regname.m_param == "wakeen")
    {
        print("WAKEEN = $%x\n\r", codec.read16(HDACodec::WAKEEN16));
        return true;        
    }

    if (regname.m_param == "statests")
    {
        print("STATESTS = $%x\n\r", codec.read16(HDACodec::HDACodec::STATESTS16));
        return true;        
    }

    return false;
}


struct commandDef
{
    const char *name;
    commandPrototype m_func;
};

static constexpr const std::array<commandDef, 1> g_commands {
    {{"read", cmdReadCodecRegister},
     //{"write", cmdTest},
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
