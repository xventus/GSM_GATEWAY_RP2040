
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   terminal_msg.h
/// @author Petr Vanek

#pragma once

#include <stdio.h>
#include <string>


using namespace std::literals;

enum class TerminalMessageType
{
	none,		/// none
    receive,	///< receive char, from Application ISR  -> terminal task 
	rtcset, 	///< from Gsm task -> terminal task 
	clearAllAck, ///< from Output task -> terminal task, ack clear output
	readAllAck,   ///< from Output task -> terminal task, ack read output

};


struct TerminalMessage
{
    TerminalMessageType  _messageType{TerminalMessageType::none};
    char            	 _c{'\0'};
    uint64_t         	 _value{0};
};