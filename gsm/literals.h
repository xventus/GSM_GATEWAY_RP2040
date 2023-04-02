//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   literals.h
/// @author Petr Vanek

#pragma once

#include <stdio.h>
#include <string_view>
#include "pico/stdlib.h"

class literals
{
public:

    //---> BEGIN ENU LOCAL 
    
    
    // LCD - 5110 display
                                                    //1234567890
    static constexpr const char *welcome            {" GSM & GPS"};
    static constexpr const char *lghwinit           {"HW init"};
    static constexpr const char *lgwait             {"waiting ... "};
    static constexpr const char *lgmodeminit        {"modem init"};
    static constexpr const char *gsmError           {">error "};   
    static constexpr const char *gsmOK              {">ready"};
    static constexpr const char *pinError           {"UNLOCK PIN"};   
    static constexpr const char *pinOK              {">pin OK"};
    static constexpr const char *simError           {"SIM ERROR"}; 
    static constexpr const char *simError2          {"SIM ERROR 2"};   
    static constexpr const char *simOK              {">sim OK"};
    static constexpr const char *registrationError  {"network ERR"};
    static constexpr const char *callerIError       {"CALLID ERROR"};   
    static constexpr const char *callerIDOK         {">call ID OK"};
    static constexpr const char *gsmTimeError       {"GSM error  "};   
    static constexpr const char *gpsTimeError       {"GPS error  "};
    static constexpr const char *gsmTimeOK          {" GSM  "};   
    static constexpr const char *gpsTimeOK          {" GPS  "};
    static constexpr const char *ring               {"RING       "};
    static constexpr const char *learning           {"CALL ME!"};
    static constexpr const char *registration       {"Registered: "}; 
    static constexpr const char *empty              {"           "};
    static constexpr const char *master             {"as master"};
    static constexpr const char *simplyOK           {"OK"};          
    static constexpr const char *simplyERROR        {"ERROR"};       
    static constexpr const char *smsCommand         {"SMS command:"};
    static constexpr const char *fullERROR          {"FULL storage"};
    static constexpr const char *output             {"O: "};       
    
    // SMS responses 
    static constexpr const char *smsNone            {"allowed commands:"};
    static constexpr const char *smsFULL            {"cannot be added, positions occupied"};
    static constexpr const char *smsAdd             {"Command accepted. The new user must call the gateway."};
    static constexpr const char *smsAccepted        {"Command accepted."};
    static constexpr const char *smsrAccepted       {"The registration of the phone number was successful."};
    static constexpr const char *smsFailed          {"Execution of the command failed. "};
    static constexpr const char *smsrFailed         {"Registration failed. "};
    static constexpr const char *smsCommanders      {"List of operators: "};
    static constexpr const char *smsCOutputs        {"Output states: "};

    //<--- END OF ENU LOCAL

    // internal - do not modify
    
    static constexpr const char *tsk_oututs{"OUTTSK"};
    static constexpr const char *tsk_lcd{"LCDTSK"};
    static constexpr const char *tsk_led{"LEDTSK"};
    static constexpr const char *tsk_gsm{"GSMTSK"};
    static constexpr const char *tsk_term{"TERMTSK"};
    static constexpr const char *tmr_gsm{"GSMTMR"};
    
    // serial line
    static constexpr const char *separator{"----------------------------------"};
    static constexpr const char *header{"Task         Runtime            %%"};

};