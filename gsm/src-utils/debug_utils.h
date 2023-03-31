//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   debug_utils.h
/// @author Petr Vanek

#pragma once

#include <memory>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "pico/types.h"

/// Insert formated hexdump into output stream.
template <class Tdata>
class HexDumper
{
private:
    const Tdata &_data;
    size_t _columns;

public:
    HexDumper(const Tdata &data, size_t columns = 16) : _data(data), _columns(columns) {}

    friend std::ostream &operator<<(std::ostream &os, const HexDumper &tx)
    {
        std::string line;
        size_t pos, col, len;
        char ch;

        if (tx._columns != 0)
        {

            std::ios_base::fmtflags original_flags = os.flags();

            len = tx._data.size();
            pos = 0;
            for (;;)
            {

                os << std::hex << std::setw(6) << std::setfill(std::ostream::char_type('0')) << (pos / tx._columns * tx._columns) << ": ";
                os.flags(original_flags);

                // HEX view
                col = 0;
                while ((col < tx._columns) && ((pos + col) < len))
                {
                    os << std::hex << std::setw(2) << std::setfill(std::ostream::char_type('0')) << (unsigned int)((unsigned char)tx._data.at(pos + col)) << " ";
                    col++;
                }
                os.flags(original_flags);
                while (col < tx._columns)
                {
                    os << "   ";
                    col++;
                }
                os.flags(original_flags);

                // ASCII view
                os << "[ ";
                col = 0;
                while ((col < tx._columns) && ((pos + col) < len))
                {
                    ch = tx._data.at(pos + col);
                    if ((ch < 32) || (ch >= 127))
                        ch = '.';
                    os << ch;
                    col++;
                }
                os.flags(original_flags);
                while (col < tx._columns)
                {
                    os << " ";
                    col++;
                }
                os.flags(original_flags);
                os << " ]" << std::endl;

                // move over line, break the loop if done
                pos += tx._columns;
                if (pos >= len)
                    break;
            }

            os.flags(original_flags);
        }

        return os;
    }
};

/**
 * @brief a set of utilities for debugging and tuning the interface
 *
 */
class DebugUtils
{

public:
    /**
     * @brief Prints contnet of datetime_t
     *
     * @param dt rp2040 Datetime structure
     */
    static void printDatetime(const datetime_t &dt)
    {
        const char *dow[7] = {"Sun\0", "Mon\0", "Tue\0", "Wed\0", "Thu\0", "Fri\0", "Sat\0"};
        printf("%02d:%02d:%02d  %02d/%02d/%04d [%s]\n", dt.hour, dt.min, dt.sec, dt.day, dt.month, dt.year, (dt.dotw < 7) ? dow[dt.dotw] : "Err");
    }

    template <typename... Args>
    static void debugMsg(const char *msg, Args... args)
    {
        printf(msg, args...);
    }
};

#define DEBUG 0

// DebugUtils::debugMsg("%s[%d] %s(): ", __FILE__, __LINE__, __FUNCTION__);
// DebugUtils::debugMsg("%s(): ", __FUNCTION__);

#define dbgPrint(...)                  \
    DebugUtils::debugMsg(__VA_ARGS__); \
    DebugUtils::debugMsg("\n")

#if DEBUG
#define dbgLog(...)                               \
    DebugUtils::debugMsg("%s(): ", __FUNCTION__); \
    DebugUtils::debugMsg(__VA_ARGS__);            \
    DebugUtils::debugMsg("\n")
#else
#define dbgLog(...)
#endif
