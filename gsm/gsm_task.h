//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   gsm_task.h
/// @author Petr Vanek

#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <memory>
#include <string>
#include <string_view>
#include "rptask.h"
#include "src-gsm/gsm.h"
#include "hardware.h"
#include "gsm_message.h"
#include "serial_impl.h"
#include "lcd_message.h"
#include "commanders.h"

/**
 * @brief GSM module task - encapsulates the complete work with GSM modem, getting status, reading commands, etc. 
 *
 */
class GSMTask : public RPTask
{

public:
	GSMTask();
	virtual ~GSMTask();

	virtual bool init(const char *name, UBaseType_t priority = tskIDLE_PRIORITY, const configSTACK_DEPTH_TYPE stackDepth = configMINIMAL_STACK_SIZE) override;

	void message(const GSMMessage &msg, bool isr);

protected:

	/**
	 * @brief GSM modem processing loop
	 * 
	 */
	void loop() override;

	/**
	 * @brief hardware error or SIM error, requires service intervention
	 *
	 */
	void death();

	/**
	 * @brief sending a message for the LCD job
	 * 
	 * @param tp 
	 * @param value 
	 */
	void valueStatus(LCDMessageType tp, int32_t value = 0);

	/**
	 * @brief sending a message for the LCD job
	 * 
	 * @param tp 
	 * @param msg 
	 * @param init 
	 */
	void sendTypeMessage(LCDMessageType tp, const char *msg, bool init = true);

	/**
	 * @brief nastavenich zakladnich vlastnosti modemu
	 * 
	 * @return true  - success
	 * @return false - failed
	 */
	bool simFirstInit();

	/**
	 * @brief processing of cyclical data displays from the modem
	 * 
	 * @param msg 
	 */
	bool processMessageView(const GSMMessage& msg);

	/**
	 * @brief checks GSM modem states
	 * 
	 */
	bool processGSMStatus();

	/**
	 * @brief An incoming SMS is processed if it is from an authorized number
	 * 
	 * @param msg - message content
	 * @param id - caller ID - phone number
	 * @param tmx  - timestamp 
	 */
	void smsOperation(std::string msg, std::string id, const datetime_t& tmx);

	/**
	 * @brief processing of the callback as a registration, if enabled
	 * 
	 * @param callerId 
	 */
	void ringOperation(std::string_view callerId);

	/**
	 * @brief start of operation view
	 * 
	 */
	void startView();

	/**
	 * @brief SMS replay
	 * 
	 * @param r - message type
	 * @param id - recepient number
	 */
	void sendSMSReply(GSMMessageType r, std::string_view id);

	
private:
	const uint32_t	_maxfails{5};	///< numbers of modem fails communication before restart
	uint32_t _failcnt{0};			///< numbers of failes
	QueueHandle_t _queueGSM;		///< RTOS queue of requests
	gsm::SerialImpl _serial;		///< hardware-dependent implementation
	gsm::GSM _gsm;					///< gsm modem instance
	bool _statusBlocker{false}; 	///< round robin status reader active
	bool _learning{false};			///< waiting for ring learning
	Commanders  _commander;			///< collected all those who have the power to control the GSM gate 
	std::string _lastOut;			///< last state of outputs
};
