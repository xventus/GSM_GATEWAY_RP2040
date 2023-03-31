//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   serial_modem_intf.h
/// @author Petr Vanek

#pragma once

#include <inttypes.h>

namespace gsm {


class ISerialModem {
public:

    virtual bool isReadable() = 0;
    virtual bool isWritable() = 0;
    virtual void putch(char c) = 0;
    virtual char getc() = 0;
    virtual void delay(uint16_t ms) = 0;
    virtual uint64_t getus() = 0;

    virtual void hardwareInit() = 0; 
    virtual void modemInit() = 0; 
};

} //namespace gsm 