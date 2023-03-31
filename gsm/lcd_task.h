//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   lcd_task.h
/// @author Petr Vanek

#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <memory>
#include "rptask.h"

#include "src-lcd5110/lcd5110.h"
#include "hardware.h"
#include "lcd_message.h"

/**
 * @brief Task for Nokia 5110 display
 *
 */
class LCDTask : public RPTask
{

public:
	LCDTask();
	virtual ~LCDTask();

	virtual bool init(const char *name, UBaseType_t priority = tskIDLE_PRIORITY, const configSTACK_DEPTH_TYPE stackDepth = configMINIMAL_STACK_SIZE) override;

	void message(const LCDMessage &msg, bool isr);

protected:
	void loop() override;

private:
	QueueHandle_t _queueDisplay;
};
