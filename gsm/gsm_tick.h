//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   gsm_tick.h
/// @author Petr Vanek

#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <memory>
#include "rptimer.h"

/**
 * @brief heart beat LED info
 * 
 */
class GSMTick : public RPTimer
{

public:
	GSMTick(){};
	virtual ~GSMTick(){};
	bool init();
	void clear();

protected:
	void loop() override;

private:
	int _looper{0};
	bool _initial{true};
};
