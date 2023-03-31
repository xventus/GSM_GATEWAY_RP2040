//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   sms_command_analyzer.h
/// @author Petr Vanek

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>       
#include <string_view>
#include <tuple>
#include "gsm_message.h"

class SmsCommandAnalyzer
{
public:

    /**
     * @brief check if the incoming SMS contains a control command
     * 
     * @param content - SMS contnet
     * @return GSMMessageType 
     */
    static  std::tuple<GSMMessageType, uint32_t, bool> analyze(std::string_view content); 

    static std::string listOfCommands();

    
};
