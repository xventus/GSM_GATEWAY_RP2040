
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   rptask.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "led_task.h"
#include "pico/stdlib.h"

LedTask::LedTask(uint8_t pin) : _pin(pin) { 
	_queue = xQueueCreate( 2 , sizeof(uint32_t));
}

LedTask::~LedTask() {
	done();
	if (_queue) vQueueDelete(_queue);
}


void LedTask::loop(){
	gpio_init(_pin);
	gpio_set_dir(_pin, GPIO_OUT);

	auto delay = _defaultTick;
	while (true) { // Loop forever
		uint32_t req;
		auto res = xQueueReceive(_queue, (void *)&req, 0);
		if (res == pdTRUE){ 
			delay = req / portTICK_PERIOD_MS;
		}

		gpio_put(_pin, 1);
		vTaskDelay(delay);
		gpio_put(_pin, 0);
		vTaskDelay(delay);

	}
 }

 void LedTask::delay(uint32_t delayInMs) {
	if (_queue) {
		xQueueSendToBack(_queue, (void *)&delayInMs, 0);
 	}
 }