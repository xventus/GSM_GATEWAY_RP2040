
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   lcd_message.h
/// @author Petr Vanek

#pragma once

#include <stdio.h>
#include <string>


using namespace std::literals;

enum class LCDMessageType
{
    init,               ///< intitialise screen
    signal,             ///< signal intensity
    provider,           ///< GSM network name
    idcaller,           ///< phone number
    date,               ///< date from gps/gsm
    time,               ///< time from gps/gsm
    status,             ///< print text as log
    number,             ///< print text as log
    gsmErrorModem,      ///< gsm error 
    backlon,            ///< back light on
    backloff            ///< back light off
};


/**
 * @brief message for controlling the LCD display
 * 
 */
struct LCDMessage
{
    LCDMessageType  _messageType{LCDMessageType::status};
    char            _message[12];
    int32_t         _value{0};
};