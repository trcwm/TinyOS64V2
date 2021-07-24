/*

   Tiny printf for embedded systems

   Niels A. Moseley
   Copyright (c) 2017-2021 Moseley Instruments
   All rights reserved

*/

#include <cstdarg>
#include <cstdint>
#include "myprint.h"


namespace mi 
{
    /** base class to provide output */
    //class PutBase
    //{
    //    public:
    //        virtual void putc(wchar_t c) = 0;
    //};

    class PutLimitedString
    {
        public:
            PutLimitedString(wchar_t *buffer, size_t bytes) : m_buffer(buffer), m_maxIdx(bytes-1)
            {
                m_idx = 0;
            }

            void putc(wchar_t c)
            {
                if (m_idx < m_maxIdx)
                {
                    m_buffer[m_idx++] = c;
                }
            }

            void terminate()
            {
                m_buffer[m_idx] = 0;
            }

            size_t getLength() const
            {
                return m_idx;
            }

        protected:
            wchar_t *m_buffer;
            size_t m_idx;
            size_t m_maxIdx;
    };

}

/** convert a 64-bit number to an 16-digit ASCII hex number */
static void toHex(mi::PutLimitedString &output, uint64_t val)
{
    constexpr const uint32_t digits = 16;
    for(uint32_t i=0; i<digits; i++)
    {
        uint32_t digit = (val >> ((digits-i-1)*4)) & 0x0F;
        if (digit <= 9)
            output.putc(L'0'+digit);
        else
            output.putc(L'A'+digit-10);
    }
}

static void toHex32(mi::PutLimitedString &output, uint32_t val)
{
    constexpr const uint32_t digits = 8;
    for(uint32_t i=0; i<digits; i++)
    {
        uint32_t digit = (val >> ((digits-i-1)*4)) & 0x0F;
        if (digit <= 9)
            output.putc(L'0'+digit);
        else
            output.putc(L'A'+digit-10);
    }
}

/** convert a 32-bit number to a 10-digit ASCII number */
static size_t toUDec(mi::PutLimitedString &output, uint32_t val)
{
    size_t chars = 0;
    uint8_t digits[10] = {0,0,0,0,0,0,0,0,0,0};
    for(uint32_t s=0; s<32; s++)
    {
        uint8_t overflow = (val & 0x80000000) ? 1 : 0;
        val <<= 1;
        for(uint32_t d=0; d<10; d++)
        {
            digits[d] = (digits[d] << 1) | overflow;
            if (digits[d] >= 10)
            {
                overflow = 1;
                digits[d] -= 10;
            }
            else
            {
                overflow = 0;
            }
        }
    }

    // strip leading zeros
    bool leadingZero = true;
    for(uint32_t i=0; i<10; i++)
    {
        if ((digits[9-i] == 0) && (leadingZero))
        {
            // do not print leading zeros
            chars++;
            //output.putc(L' ');
        }
        else
        {
            chars++;
            output.putc(digits[9-i] + L'0');
            leadingZero = false;
        }
    }

    // check if we've had only leading zeros and
    // thus haven't printed anything.. :)
    if (leadingZero)
    {
        chars++;
        output.putc(L'0');
    }

    return chars;
}

static void format(mi::PutLimitedString &output, const char *fmt, va_list va)
{
    size_t tmp;
    uint32_t vtmp;
    char *charptr;
    char ch;

    while((ch=*(fmt++)) !=0)
    {
        if (ch != L'%')
        {
            output.putc(ch);
        }
        else
        {
            ch=*fmt++;
            switch(ch)
            {
                case L'X':
                    toHex(output, va_arg(va, uint64_t));
                    break;
                case L'x':
                    toHex32(output, va_arg(va, uint32_t));
                    break;                    
                case L'u':
                    tmp = toUDec(output, va_arg(va, uint32_t));
                    break;
                case L'd':
                    vtmp =  va_arg(va, int32_t);
                    if ((vtmp & 0x80000000) != 0)
                    {
                        // sign bit, we have a negative number
                        vtmp = ~vtmp;   // invert bits
                        vtmp++;         // add one

                        output.putc(L'-');
                        tmp = toUDec(output, vtmp);
                    }
                    else
                    {
                        tmp = toUDec(output, vtmp);
                    }
                    break;
                case L'c':
                    ch = va_arg(va, int);
                    output.putc(ch);
                    break;
                case L's':
                    charptr = va_arg(va, char*);
                    while(*charptr != 0)
                    {
                        output.putc(*charptr++);
                    }
                    break;
                case L'%':
                    output.putc(L'%');
                    break;
                default:
                    output.putc(L'?');
                    break;
            }
        }
    }
}

#if 0
static void format(mi::PutLimitedString &output, const wchar_t *fmt, va_list va)
{
    size_t tmp;
    uint32_t vtmp;
    wchar_t *charptr;
    wchar_t ch;

    while((ch=*(fmt++)) !=0)
    {
        if (ch != L'%')
        {
            output.putc(ch);
        }
        else
        {
            ch=*fmt++;
            switch(ch)
            {
                case L'X':
                    toHex(output, va_arg(va, uint64_t));
                    break;
                case L'x':
                    toHex32(output, va_arg(va, uint32_t));
                    break;                    
                case L'u':
                    tmp = toUDec(output, va_arg(va, uint32_t));
                    break;
                case L'd':
                    vtmp =  va_arg(va, int32_t);
                    if ((vtmp & 0x80000000) != 0)
                    {
                        // sign bit, we have a negative number
                        vtmp = ~vtmp;   // invert bits
                        vtmp++;         // add one

                        output.putc(L'-');
                        tmp = toUDec(output, vtmp);
                    }
                    else
                    {
                        tmp = toUDec(output, vtmp);
                    }
                    break;
                case L'c':
                    ch = va_arg(va, int);
                    output.putc(ch);
                    break;
                case L's':
                    charptr = va_arg(va, wchar_t*);
                    while(*charptr != 0)
                    {
                        output.putc(*charptr++);
                    }
                    break;
                case L'%':
                    output.putc(L'%');
                    break;
                default:
                    output.putc(L'?');
                    break;
            }
        }
    }
}
#endif

size_t snprintf(wchar_t *s, size_t n, const char *fmt, va_list va)
{
    mi::PutLimitedString outString(s, n);

    format(outString, fmt,va);
    outString.terminate();

    return outString.getLength();
}
