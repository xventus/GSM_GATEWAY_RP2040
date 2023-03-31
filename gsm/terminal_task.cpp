
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   terminal_task.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "terminal_task.h"
#include "pico/stdlib.h"
#include "src-utils/time_utils.h"
#include "terminal_msg.h"
#include "output_msg.h"
#include "application.h"

TerminalTask::TerminalTask()
{
	_queue = xQueueCreate(10, sizeof(TerminalMessage));
	_timebase.init();
}

TerminalTask::~TerminalTask()
{
	done();
	if (_queue)
		vQueueDelete(_queue);
}

void TerminalTask::message(const char ch, bool isr)
{
	TerminalMessage tm;
	tm._c = ch;
	tm._messageType = TerminalMessageType::receive;
	if (_queue)
	{
		(isr) ? xQueueSendToBackFromISR(_queue, (void *)&tm, 0) : xQueueSendToBack(_queue, (void *)&tm, 0);
	}
}

void TerminalTask::message(const TerminalMessage &msg, bool isr)
{
	if (_queue)
	{
		(isr) ? xQueueSendToBackFromISR(_queue, (void *)&msg, 0) : xQueueSendToBack(_queue, (void *)&msg, 0);
	}
}

void TerminalTask::loop()
{
	OutputMsg msgx;
	TerminalMessage req;

	while (true)
	{ // Loop forever

		auto res = xQueueReceive(_queue, (void *)&req, (TickType_t)50 / portTICK_PERIOD_MS);
		if (res == pdTRUE)
		{
			if (req._messageType == TerminalMessageType::receive)
			{
				if (_proto.parse(req._c))
				{

					switch (_proto.getCommand())
					{

					case TerminalProto::Cmd::ascitime:
						if (_timebase.isValid())
						{
							// real time is valid and print as ascii string
							auto tmbs = _timebase.getTimeDate();
							std::string tmstr;
							tmstr = TimeUtils::timeToString(tmbs);
							tmstr += " ";
							tmstr += TimeUtils::dateToString(tmbs);
							auto response = TerminalProto::makeResponse(_proto._address, _proto.isChecksumRequired() ? TerminalProto::_asciiTimeChck : TerminalProto::_asciiTime, tmstr, _proto.isChecksumRequired());
							printf("%s\r\n", response.c_str());
						}
						else
						{
							// not valid time
							auto response = TerminalProto::makeResponse(_proto._address, _proto.isChecksumRequired() ? TerminalProto::_asciiTimeChck : TerminalProto::_asciiTime, _proto.isChecksumRequired());
							printf("%s\r\n", response.c_str());
						}
						break;

					case TerminalProto::Cmd::time:

						if (_timebase.isValid())
						{
							// real time is valid
							auto tmbs = _timebase.getTimeDate();
							auto unixtime = TimeUtils::makeUnixTime(tmbs);
							auto response = TerminalProto::makeResponse(_proto._address, _proto.isChecksumRequired() ? TerminalProto::_timeChck : TerminalProto::_time, unixtime, _proto.isChecksumRequired());
							printf("%s\r\n", response.c_str());
						}
						else  
						{
							// not valid time - empty vlue
							auto response = TerminalProto::makeResponse(_proto._address, _proto.isChecksumRequired() ? TerminalProto::_timeChck : TerminalProto::_time, _proto.isChecksumRequired());
							printf("%s\r\n", response.c_str());
						}

						break;

					case TerminalProto::Cmd::read:
						// send message to Output task

						msgx._output = 0;
						msgx._value = false;
						msgx._messageType = OutputTypeMsg::readallTerm;
						Application::getInstance()->getOutputTask()->message(msgx, false);
						break;

					case TerminalProto::Cmd::clear:

						msgx._output = 0;
						msgx._value = false;
						msgx._messageType = OutputTypeMsg::writeAllOffTerm;
						Application::getInstance()->getOutputTask()->message(msgx, false);
						break;

						break;
					}
				}
			}
			else if (req._messageType == TerminalMessageType::rtcset)
			{
				//receive from GSM task with valid time timestamp
				datetime_t tm;
				TimeUtils::breakUnixTime(req._value, tm);
				_timebase.updateTime(tm);
			}
			else if (req._messageType == TerminalMessageType::clearAllAck)
			{
				// received from Output task  - ACK of OutputTypeMsg::writeAllOffTerm
				// It is assumed that there will be no further news of a different type. Because the master controls the communication.
				auto response = TerminalProto::makeResponse(_proto._address, _proto.isChecksumRequired() ? TerminalProto::_clearChck : TerminalProto::_clear, (uint32_t)req._value, _proto.isChecksumRequired());
				printf("%s\r\n", response.c_str());
			}
			else if (req._messageType == TerminalMessageType::readAllAck)
			{
				// received from Output task  - ACK of OutputTypeMsg::readallTerm
				// It is assumed that there will be no further news of a different type. Because the master controls the communication.
				auto response = TerminalProto::makeResponse(_proto._address, _proto.isChecksumRequired() ? TerminalProto::_readChck : TerminalProto::_read, (uint32_t)req._value, _proto.isChecksumRequired());
				printf("%s\r\n", response.c_str());
			}
		}
	}
}
