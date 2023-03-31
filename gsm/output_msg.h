//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   output_msg.h
/// @author Petr Vanek

#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <memory>

/**
 * @brief message type for output control
 *
 */
enum class OutputTypeMsg
{
	writeone,		 ///< set pin to ON or OFF
	writeAllOff,	 ///< all pins to OFF
	readall,		 ///< for UI view
	readallTerm,	 ///< for terminal control
	writeAllOffTerm, ///< for terminal control
	none
};

/**
 * @brief message for Output
 *
 */
struct OutputMsg
{
public:
	OutputTypeMsg _messageType{OutputTypeMsg::none}; ///< message type
	uint32_t _output{0};							 ///< identify output mask or pin
	bool _value{false};								 ///< state
};
