
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   gsm_tick.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "gsm_tick.h"
#include "literals.h"
#include "application.h"
#include "gsm_message.h"


bool GSMTick::init() {
    return RPTimer::init(literals::tmr_gsm, pdMS_TO_TICKS(500), true);
}

void GSMTick::clear() {
		_initial = true;
		_looper = 0;
		changePeriod(pdMS_TO_TICKS(500),100); 
}

void GSMTick::loop() {
   
    GSMMessage msg;
    msg._messageType = GSMMessageType::view;
    msg._value = _looper;
    Application::getInstance()->getGSMTask()->message(msg, false);
    _looper++;
    if (_looper > 5) {
        _looper = 0;
        if (_initial) {
            _initial = false;
            changePeriod(pdMS_TO_TICKS(5000),100);
        }
    }
}
