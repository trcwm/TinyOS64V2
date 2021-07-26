#pragma once
#include <string_view>
#include <cstdlib>

constexpr bool strncmp(const char *s1, const char *s2, size_t bytes)
{
    for(size_t i=0; i<bytes; i++)
    {
        if (s1[i] != s2[i])
            return false;
        
        // detect early end of strings
        if ((s1[i] == 0) && (s2[i] == 0))
            return true;
    }
    return true;
}

constexpr bool strncmp(const std::string_view &s1, const char *s2)
{
    for(size_t i=0; i<s1.size(); i++)
    {
        if (s1[i] != s2[i])
            return false;
    }
    return true;    
}

constexpr bool strncmp(const std::string_view &s1, const std::string_view &s2)
{
    if (s1.size() != s2.size())
        return false;

    for(size_t i=0; i<s1.size(); i++)
    {
        if (s1[i] != s2[i])
            return false;
    }
    return true;    
}
