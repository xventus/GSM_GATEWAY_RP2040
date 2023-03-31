
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   gsm_message.h
/// @author Petr Vanek

#pragma once

#include <algorithm>
#include <inttypes.h>
#include <string>
#include <string_view>
#include <typeinfo>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "hardware.h"

using namespace std::literals;

/**
 * @brief enum for sms and status handling
 *
 */
enum class GSMMessageType
{
    view,       ///< view display step
    none,       ///< N/A, unknown commnad... aka help
    output1ON,  ///< output 1 -> ON
    output1OFF, ///< output 1 -> OFF
    output2ON,  ///< output 2 -> ON
    output2OFF, ///< output 2 -> OFF
    output3ON,  ///< output 3 -> ON
    output3OFF, ///< output 3 -> OFF
    output4ON,  ///< output 4 -> ON
    output4OFF, ///< output 4 -> OFF
    output5ON,  ///< output 5 -> ON
    output5OFF, ///< output 5 -> OFF
    output6ON,  ///< output 6 -> ON
    output6OFF, ///< output 6 -> OFF
    output7ON,  ///< output 7 -> ON
    output7OFF, ///< output 7 -> OFF
    alloff,     ///< all OFF
    utctime,    ///< UTC time from GPS or GSM
    accepted,   ///< accept commnad
    failed,     ///< refused / error
    add,        ///< add new caller id - only elevated user (1st phone number)
    del,        ///< delete caller id  - only elevated user (1st phone number) - not implemented now
    list,       ///< phone number list - only elevated user (1st phone number)
    full,       ///< full commander list
    raccepted,  ///< registration accepted
    rfailed,    ///< registration failed
    state       ///< output state
};

/**
 * @brief exchange structure for messaging
 *
 */
struct GSMMessage
{
    GSMMessageType _messageType{GSMMessageType::none};
    int64_t _value{0};
    char _message[12];
};

/**
 * @brief structure for each SMS command definition
 * 
 */
struct SMSCommands
{
public:
    SMSCommands(std::string_view cmd, GSMMessageType msg, uint32_t pin, bool adm, bool onOff) : _cmd(cmd), _msgT(msg), _pin(pin), _adm(adm), _onOff(onOff){};
    std::string_view    _cmd;               ///< command - unigue string - case insensitive
    GSMMessageType      _msgT;              ///< message type
    uint32_t            _pin{UINT32_MAX};   ///< assigned pin 0-n - real HW pin
    bool                _adm{false};        ///< is master user only commad
    bool                _onOff{false};      ///< output is ON or OFF, when PIN is defined 
};

/**
 * @brief Array of SMS commands
 *
 */
const SMSCommands gsmCommandArray[]{
    // each output - ON / OFF
    {"1 ON"sv, GSMMessageType::output1ON,   AUX1_PIN, false, true},
    {"1 OFF"sv, GSMMessageType::output1OFF, AUX1_PIN, false, false},
    {"2 ON"sv, GSMMessageType::output2ON,   AUX2_PIN, false, true},
    {"2 OFF"sv, GSMMessageType::output2OFF, AUX2_PIN, false, false},
    {"3 ON"sv, GSMMessageType::output3ON,   AUX3_PIN, false, true},
    {"3 OFF"sv, GSMMessageType::output3OFF, AUX3_PIN, false, false},
    {"4 ON"sv, GSMMessageType::output4ON,   AUX4_PIN, false, true},
    {"4 OFF"sv, GSMMessageType::output4OFF, AUX4_PIN, false, false},
    {"5 ON"sv, GSMMessageType::output5ON,   AUX5_PIN, false, true},
    {"5 OFF"sv, GSMMessageType::output5OFF, AUX5_PIN, false, false},
    {"6 ON"sv, GSMMessageType::output6ON,   AUX6_PIN, false, true},
    {"6 OFF"sv, GSMMessageType::output6OFF, AUX6_PIN, false, false},
    {"7 ON"sv, GSMMessageType::output7ON,   AUX7_PIN, false, true},
    {"7 OFF"sv, GSMMessageType::output7OFF, AUX7_PIN, false, false},

    // common
    {"ALL OFF"sv, GSMMessageType::alloff, UINT32_MAX, false, false},
    {"STATE"sv, GSMMessageType::state, UINT32_MAX, false, false},

    // phone number management
    {"ADD"sv, GSMMessageType::add, UINT32_MAX, true, false},
    {"LIST"sv, GSMMessageType::list, UINT32_MAX, true, false}

};