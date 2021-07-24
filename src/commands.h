#pragma once

#include <string_view>
#include "hdacodec.h"

typedef bool (*commandPrototype)(HDACodec &codec, const std::string_view &params);

class Commands
{
public:
    Commands(HDACodec &codec) : m_codec(codec) {}

    bool execute(const std::string_view &line);

protected:
    constexpr bool isAlpha(char c) const
    {
        if ((c>='a') && (c<='z')) return true;
        if ((c>='A') && (c<='Z')) return true;
        return false;
    }

    constexpr bool isNumeric(char c) const
    {
        if ((c>='0') && (c<='9')) return true;
        return false;
    }

    constexpr bool isHex(char c) const
    {
        if ((c>='0') && (c<='9')) return true;
        if ((c>='a') && (c<='f')) return true;
        if ((c>='A') && (c<='F')) return true;
        return false;
    }

    std::string_view getCommand(const std::string_view &line) const;

    HDACodec &m_codec;
};

