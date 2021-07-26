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

    std::string_view getCommand(const std::string_view &line) const;

    HDACodec &m_codec;
};

