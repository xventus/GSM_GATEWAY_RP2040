//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   terminal_task.h
/// @author Petr Vanek

#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "terminal_msg.h"
#include "rptask.h"
#include "terminal_proto.h"
#include "src-utils/time_base.h"


/**
 * @brief Terminal interface for communication via RS485
 * 
 */
class TerminalTask : public RPTask
{
public:
	TerminalTask();
	virtual ~TerminalTask();

	void message(const TerminalMessage &msg, bool isr);
	void message(const char ch, bool isr);

protected:
	void loop() override;

private:
	TerminalProto _proto;
	QueueHandle_t _queue;
	TimeBase _timebase;
};
