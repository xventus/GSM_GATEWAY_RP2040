//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   task.h
/// @author Petr Vanek

#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "hardware.h"
#include "rptask.h"


class LedTask : public RPTask
{
public:
	LedTask(uint8_t pin = HEART_BEAT_LED);
	virtual ~LedTask();
	void delay(uint32_t delayInMs);

protected:
	void loop() override;

private:
	const uint32_t  _defaultTick{1000};
	uint8_t 		_pin{HEART_BEAT_LED};
	QueueHandle_t 	_queue;
};
