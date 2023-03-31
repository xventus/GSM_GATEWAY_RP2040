
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   terminal_proto.h
/// @author Petr Vanek

#pragma once

#include <stdio.h>
#include <string>
#include <string_view>
#include "src-utils/debug_utils.h"

using namespace std::literals;

/*
       Terminal protocol:

       Request:   Address|Commnad|;<Checksum>
                  Address|Commnad|numeric value;<Checksum>

       Address - ASCII char e.g.  'T' - terminal
       Commnad - ASCII uppercase is command withouth checkusm
               - ASCII lowercase with checksum
       Checkusum - ASCII representation of checksum

       commands:
           R, r - read output status
           C, c - clear output status  (0 - 255) set the bits to be reset
           T, t - get time

       Response:
           Address|Command|value number;<Checksum>
           The address and command is repeated in the reply

       the semicolon is always the separator between the value and the optional checksum
*/

class TerminalProto
{

public:    

    static const char _address{'T'};
    static const char _readChck{'r'};
    static const char _read{'R'};
    static const char _timeChck{'t'};
    static const char _time{'T'};
    static const char _clearChck{'c'};
    static const char _clear{'C'};
    static const char _asciiTimeChck{'a'};
    static const char _asciiTime{'A'};

    enum class Cmd
    {
        read,
        clear,
        time,
        ascitime,
        none
    };

    static char calculateChceckSum(std::string_view m)
    {
        std::string rc;
        uint8_t chck = 0;
        
        for (const auto & s : m) 
        {
            chck ^= (uint8_t)s;
        }
        
        return convertTochar(chck);
    } 

    static std::string makeResponse(const char address, const char command, uint32_t value, bool chcek) {
        std::string rc;
        uint8_t chck = 0;
        
        rc += address;
        rc += command;
        rc += std::to_string(value);
        rc += ";";
        
        if (chcek) {
            chck = calculateChceckSum(rc);
            rc += convertTochar(chck);
        }

        return rc;
    }


    static std::string makeResponse(const char address, const char command, std::string_view value, bool chcek) {
        std::string rc;
        uint8_t chck = 0;
        
        rc += address;
        rc += command;
        rc += value;
        rc += ";";
        
        if (chcek) {
            chck = calculateChceckSum(rc);
            rc += convertTochar(chck);
        }

        return rc;
    }

    static std::string makeResponse(const char address, const char command, bool chcek) {
        std::string rc;
        uint8_t chck = 0;
        
        rc += address;
        rc += command;
        rc += ";";
        
        if (chcek) {
            chck = calculateChceckSum(rc);
            rc += convertTochar(chck);
        }

        return rc;
    }

    static char convertTochar(uint8_t aux)
    {
        uint8_t chck = aux;
        if (chck < 33)
        {
            // smaller than !
            chck += 33;
        }
        else if (chck > 126)
        {
            // grater than ~
            chck = chck - 126;
            if (chck < 33)
                chck += 32;
            if (chck > 126)
                chck -= 32;
        }
        return (char)chck;
    }

    bool parse(char c)
    {

        bool rc = false;

        do
        {
            if (c == '\n')
                break;

            if (c == '\r')
            {
                _step = Step::address;
                _ignore = false;
                _chck = 0;
                break;
            }

            if (_ignore)
                break;

            switch (_step)
            {

            case Step::address:
                if (c == _address)
                {
                    _chck ^= c;
                    _ignore = false;
                    _step = Step::command;
                    _chceksum = false;
                    _acc.clear();
                }
                else
                {
                    _ignore = true;
                }
                break;

            case Step::command:

                _chck ^= c;
                switch (c)
                {
                case 't':
                    _cmd = Cmd::time;
                    _chceksum = true;
                    _step = Step::semicolon;
                    break;

                case 'T':
                    _cmd = Cmd::time;
                    _step = Step::semicolon;
                    break;

                case 'c':
                    _cmd = Cmd::clear;
                    _chceksum = true;
                    _step = Step::semicolon;
                    break;

                case 'C':
                    _cmd = Cmd::clear;
                    _step = Step::semicolon;
                    break;

                case 'r':
                    _cmd = Cmd::read;
                    _chceksum = true;
                    _step = Step::semicolon;
                    break;

                case 'R':
                    _cmd = Cmd::read;
                    _step = Step::semicolon;
                    break;


                case 'a':
                    _cmd = Cmd::ascitime;
                    _chceksum = true;
                    _step = Step::semicolon;
                    break;

                case 'A':
                    _cmd = Cmd::ascitime;
                    _step = Step::semicolon;
                    break;




                default:
                    _cmd = Cmd::none;
                    _ignore = true;
                }

                break;

            case Step::check:
                dbgLog("required checksum %c \n", convertTochar(_chck));
                if (true)
                {
                    if (convertTochar(_chck) == c)
                    {
                        rc = true;
                    }
                }
                break;

            case Step::semicolon:

                _chck ^= c;
                if (c == ';')
                {

                    if (!_acc.empty())
                    {
#ifdef __cpp_exceptions
                        try
                        {
                            _value = static_cast<uint32_t>(std::stoul(_acc));
                        }
                        catch (...)
                        {
                            _value = 0;
                        }
#else
                        _value = strtoul(_acc.c_str(), nullptr, 10);
#endif

                    }
                    else
                    {
                        _value = 0;
                    }

                    if (_chceksum)
                    {
                        _step = Step::check;
                    }
                    else
                    {
                        rc = true;
                    }
                }
                else
                {
                    _acc += c;
                }
                break;
            }
        }

        while (false);

        return rc;
    }

    Cmd getCommand()
    {
        return _cmd;
    }

    uint32_t getValue()
    {
        return _value;
    }

    bool isChecksumRequired() const {
        return _chceksum;
    }

private:
    enum class Step
    {
        address,   // device address
        command,   // commnad
        semicolon, // wait for delimiter
        check,     // wait for checksum
        lineend    // CR - finalize command, LF ignored
    };

    
    Step _step{Step::address};
    bool _ignore{false};
    bool _chceksum{false};
    Cmd _cmd{Cmd::none};
    uint32_t _value{0};
    std::string _acc;
    uint8_t _chck{0};
};