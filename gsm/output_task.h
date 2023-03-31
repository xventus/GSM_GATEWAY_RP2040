//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   output_task.h
/// @author Petr Vanek

#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <memory>
#include "rptask.h"
#include "hardware.h"
#include "output_msg.h"

/**
 * @brief Task for outputs control
 * 
 */
class OutputTask : public RPTask
{

public:
	OutputTask();
	virtual ~OutputTask();
    virtual bool init(const char * name, UBaseType_t priority = tskIDLE_PRIORITY, const configSTACK_DEPTH_TYPE stackDepth = configMINIMAL_STACK_SIZE) override;
	void writeToOutput(uint8_t outputId, bool on, bool isr);
	void message(const OutputMsg& msg, bool isr);

protected:
	void loop() override;

	/**
	 * @brief map from real pins position into mapped to string  1-7 
	 * 
	 * @param outputs - real pin mask position 
	 * @return std::string 
	 */
	std::string outputsToString(const uint32_t outputs);

	/**
	 * @brief map from real pins position to bit position bit 1, 2, 4, 8 .. 
	 * 
	 * @param outputs 
	 * @return uint32_t 
	 */
	uint32_t outputsToOrder(const uint32_t outputs);

private:

	uint32_t getOutputmask();
    QueueHandle_t 	_queueRequest;
   
};
