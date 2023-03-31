
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
#include "gsm_task.h"
#include "pico/stdlib.h"
#include "src-utils/debug_utils.h"
#include "src-utils/time_utils.h"
#include "literals.h"
#include "lcd_task.h"
#include "application.h"
#include "sms_command_analyzer.h"

GSMTask::GSMTask() : _gsm(_serial)
{
    _queueGSM = xQueueCreate(5, sizeof(GSMMessage));
}

// -------------------------------------------------------------------------------------------------

GSMTask::~GSMTask()
{
    done();
    if (_queueGSM)
        vQueueDelete(_queueGSM);
}

// -------------------------------------------------------------------------------------------------

void GSMTask::message(const GSMMessage &msg, bool isr)
{
    if (_queueGSM)
    {
        (isr) ? xQueueSendToBackFromISR(_queueGSM, (void *)&msg, 0) : xQueueSendToBack(_queueGSM, (void *)&msg, 0);
    }
}

// -------------------------------------------------------------------------------------------------

void GSMTask::death()
{

    Application::getInstance()->getLEDTask()->delay(100);
    while (true)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// -------------------------------------------------------------------------------------------------

void GSMTask::valueStatus(LCDMessageType tp, int32_t value)
{
    LCDMessage lcdmsg;
    lcdmsg._message[0] = '\0';
    lcdmsg._value = value;
    lcdmsg._messageType = tp;
    Application::getInstance()->getLCDTask()->message(lcdmsg, false);
};

// -------------------------------------------------------------------------------------------------

void GSMTask::sendTypeMessage(LCDMessageType tp, const char *msg, bool init)
{
    LCDMessage lcdmsg;
    lcdmsg._messageType = tp;
    lcdmsg._value = init ? 0 : 1;
    memset(&lcdmsg._message, 0, sizeof(lcdmsg._message));
    if (msg)
    {
        strncpy(lcdmsg._message, msg, sizeof(lcdmsg._message) - 1);
    }
    Application::getInstance()->getLCDTask()->message(lcdmsg, false);
};

// -------------------------------------------------------------------------------------------------

bool GSMTask::simFirstInit()
{
    bool rc = false;
    do
    {
        // chcek SIM
        auto icd = _gsm.getICCID();
        if (icd.has_value())
        {
            if (!icd.value().empty())
            {
                sendTypeMessage(LCDMessageType::status, literals::simOK, false);
            }
            else
            {
                sendTypeMessage(LCDMessageType::status, literals::simError, false);
                break;
            }
        }
        else
        {
            sendTypeMessage(LCDMessageType::status, literals::simError2, false);
            break;
        }

        // check PIN
        auto ispin = _gsm.pinNotRequired();
        if (ispin.has_value())
        {
            if (ispin.value())
            {
                sendTypeMessage(LCDMessageType::status, literals::pinOK, false);
            }
            else
            {
                sendTypeMessage(LCDMessageType::status, literals::pinError, false);
                break;
            }
        }
        else
        {
            sendTypeMessage(LCDMessageType::status, literals::pinError, false);
            break;
        }

        // set caller ID
        auto callerId = _gsm.setCallerID(true);
        if (callerId)
        {
            sendTypeMessage(LCDMessageType::status, literals::callerIDOK, false);
        }
        else
        {
            sendTypeMessage(LCDMessageType::status, literals::callerIError, false);
        }

        // delete SMS storage
        _gsm.delAllSMS();

        // registration to network
        auto isRegisterd = _gsm.isRegistered();
        if (!(isRegisterd.has_value() && isRegisterd.value() == 1))
        {
            sendTypeMessage(LCDMessageType::status, literals::registrationError, false);
            break;
        }

        rc = true;
    } while (false);
    return rc;
}

// -------------------------------------------------------------------------------------------------

bool GSMTask::processMessageView(const GSMMessage &msg)
{
    bool rc = true;

    if (msg._value == 0)
    {
        // operator name
        auto oper = _gsm.getOperator();
        if (oper.has_value())
        {
            sendTypeMessage(LCDMessageType::provider, oper.value().c_str(), false);
            _failcnt = 0;
        } else {
             rc = false;
             _failcnt ++;
        }
    }
    else if (msg._value == 1)
    {
        // signal quality
        auto signal = _gsm.qualitySignal();
        if (signal.has_value())
        {
            valueStatus(LCDMessageType::signal, signal.value());
        } else {
            rc = false;
        }
    }
    else if (msg._value == 2)
    {
        //  RTC
        auto dm = _gsm.readRTC();
        if (dm.has_value())
        {
            if (TimeUtils::isTimestampCorrect(dm.value()))
            {
                TerminalMessage tmmsg;
                tmmsg._messageType = TerminalMessageType::rtcset;
                tmmsg._value = TimeUtils::makeUnixTime(dm.value()) ;
                Application::getInstance()->getTerminalTask()->message(tmmsg, false);
                auto timestr = TimeUtils::timeToStringShort(dm.value());
                timestr += literals::gsmTimeOK;
                sendTypeMessage(LCDMessageType::time, timestr.c_str(), false);
                auto datestr = TimeUtils::dateToString(dm.value());
                sendTypeMessage(LCDMessageType::date, datestr.c_str(), false);
            }
            else
            {
                sendTypeMessage(LCDMessageType::time, literals::gsmTimeError, false);
            }
        } 
    }
    else if (msg._value == 3)
    {
        // GNSS
        auto gnss = _gsm.gnssInfo();
        if (gnss.has_value())
        {
            auto [tmxm, fix, stat] = gnss.value();
            auto timestr = TimeUtils::timeToStringShort(tmxm);
            timestr += literals::gpsTimeOK;
            sendTypeMessage(LCDMessageType::time, timestr.c_str(), false);
            auto datestr = TimeUtils::dateToString(tmxm);
            sendTypeMessage(LCDMessageType::date, datestr.c_str(), false);
        }
        else
        {
            sendTypeMessage(LCDMessageType::time, literals::gpsTimeError, false);
        }
    }
    else if (msg._value == 4)
    {
        if (_learning)
        {
            sendTypeMessage(LCDMessageType::idcaller, literals::learning, false);
        }
    }
    else if (msg._value == 5)
    {
        if (_learning)
        {
            sendTypeMessage(LCDMessageType::idcaller, literals::learning, false);
        }
        else
        {
            OutputMsg msgx;
            msgx._output = 0;
            msgx._value = false;
            msgx._messageType = OutputTypeMsg::readall;
            Application::getInstance()->getOutputTask()->message(msgx, false);
        }

        sendTypeMessage(LCDMessageType::backloff, literals::empty, false);
    } else if (msg._value == 6) {
        //check registration to the network
        auto isRegisterd = _gsm.isRegistered();
        if (!(isRegisterd.has_value() && isRegisterd.value() == 1))
        {
            rc = false;
            _failcnt ++;
        } else {
            _failcnt = 0;
        }
    }

    return rc;
}

// -------------------------------------------------------------------------------------------------

void GSMTask::startView()
{
    // with first faster cycle
    Application::getInstance()->getGSMTick()->clear();
    // default refresh period
    Application::getInstance()->getGSMTick()->start(100);
    valueStatus(LCDMessageType::init);
    valueStatus(LCDMessageType::signal, 0);
}

// -------------------------------------------------------------------------------------------------

void GSMTask::loop()
{
    GSMMessage msg;
    LCDMessage lcdmsg;
    int32_t stpe = 0;
    
    _commander.refresh();
    if (_commander.isEmpty())
    {
        // first learning info display
        _learning = true;
    }

    _gsm.whitInfoCallback([this, &lcdmsg, &stpe](gsm::GsmInfoState nfo)
                          {
        
        switch(nfo) {

            case gsm::GsmInfoState::hwinit:
                // 1st message from init
                sendTypeMessage(LCDMessageType::status, literals::lghwinit, true); 
                stpe = 0;
            break;

            case gsm::GsmInfoState::gnss:
                // last message from init
                stpe++;
                valueStatus(LCDMessageType::number, stpe); 
            break;

            case gsm::GsmInfoState::wait:
               sendTypeMessage(LCDMessageType::status, literals::lgwait, false); 
            break;

            case gsm::GsmInfoState::ping:
               stpe++;
               valueStatus(LCDMessageType::number, stpe); 
            break;

            case gsm::GsmInfoState::rx:
                // receive data from modem
            break;

            case gsm::GsmInfoState::tx:
               // transmite data to modem
            break;
    
        } });

    // reinit cycle
    while (true)
    {
        _failcnt = 0;

        // modem initialization
        if (!_gsm.init(true))
        {
            // unrecoverable error, service intervention required
            sendTypeMessage(LCDMessageType::status, literals::gsmError, true);
            death();
        }

        // OK status
        sendTypeMessage(LCDMessageType::status, literals::gsmOK, true);

        if (!simFirstInit())
        {
            // unrecoverable error, service intervention required
            death();
        }

        // GSM & GPS modem refresher
        startView();

        // gsm cycle live
        while (true)
        {

            auto res = xQueueReceive(_queueGSM, (void *)&msg, (TickType_t)10 / portTICK_PERIOD_MS);
            if (res == pdTRUE)
            {
                if (msg._messageType == GSMMessageType::view)
                {
                    processMessageView(msg);
                }

                if (msg._messageType == GSMMessageType::state)
                {
                    _lastOut = msg._message;
                }
            }

            // gsm modem status ring, new sms ...
            processGSMStatus();

            if (_failcnt > _maxfails)  {
                // restart modem
                break;
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------

void GSMTask::ringOperation(std::string_view callerId)
{

    // learning operation
    Application::getInstance()->getGSMTick()->stop(100);
    sendTypeMessage(LCDMessageType::status, literals::registration, true);
    sendTypeMessage(LCDMessageType::status, std::string(callerId).c_str(), false);
    if (_commander.isEmpty())
    {
        sendTypeMessage(LCDMessageType::status, literals::master, false);
    }

    if (_commander.addNew(callerId))
    {
        sendTypeMessage(LCDMessageType::status, literals::simplyOK, false);
        sendSMSReply(GSMMessageType::raccepted, callerId);
    }
    else
    {
        if (_commander.isFull())
        {
            sendTypeMessage(LCDMessageType::status, literals::fullERROR, false);
            sendSMSReply(GSMMessageType::full, callerId);
        }
        else
        {
            sendTypeMessage(LCDMessageType::status, literals::simplyERROR, false);
            sendSMSReply(GSMMessageType::failed, callerId);
        }
    }

    _learning = false;
    startView();
}

// -------------------------------------------------------------------------------------------------

void GSMTask::sendSMSReply(GSMMessageType r, std::string_view id)
{
    std::string smsContent;
    std::string aux;

    
    switch (r)
    {
    case GSMMessageType::full:
        smsContent = literals::smsFULL;
        break;

    case GSMMessageType::none:
        smsContent = literals::smsNone;
        smsContent += "\n";
        smsContent += SmsCommandAnalyzer::listOfCommands();
        break;

    case GSMMessageType::list:
        smsContent = literals::smsCommanders;
        smsContent += "\n";
        aux = _commander.getList();
        smsContent += aux;
        break;

    case GSMMessageType::state:
        smsContent = literals::smsCOutputs;
        smsContent += "\n";
        smsContent += _lastOut;
        break;

    case GSMMessageType::add:
        smsContent = literals::smsAdd;
        break;

    case GSMMessageType::accepted:
        smsContent = literals::smsAccepted;
        break;

    case GSMMessageType::raccepted:
        smsContent = literals::smsrAccepted;
        break;

    case GSMMessageType::failed:
        smsContent = literals::smsFailed;
        break;

    case GSMMessageType::rfailed:
        smsContent = literals::smsrFailed;
        break;
    }

    if (!smsContent.empty())
    {
       dbgLog("SENDSMS :>%s<\n", smsContent.c_str()); 
       _gsm.sendSMS(id, smsContent);
    }
}

// -------------------------------------------------------------------------------------------------

void GSMTask::smsOperation(std::string msg, std::string id, const datetime_t &tmx)
{

    do
    {
        // determining whether a telephone number is authorised to perform an operation
        if (!_commander.isExist(id))
            break;

        Application::getInstance()->getGSMTick()->stop(100);
        sendTypeMessage(LCDMessageType::status, literals::smsCommand, true);
        sendTypeMessage(LCDMessageType::status, std::string(id).c_str(), false);

        // which operation
        auto [cmd, pin, onOff] = SmsCommandAnalyzer::analyze(msg);

        if ((cmd == GSMMessageType::add || cmd == GSMMessageType::list) && !_commander.isSupremeCommander(id))
        {                                           // not priviledged user
            sendSMSReply(GSMMessageType::none, id); // SMS:  send help;
            sendTypeMessage(LCDMessageType::status, literals::simplyOK, false);
            break;
        }

        // add new user - learning
        if (cmd == GSMMessageType::add)
        {
            if (_commander.isFull())
            {
                _learning = false;
                // SMS:  send learning  full
                sendTypeMessage(LCDMessageType::status, literals::simplyERROR, false);
                sendSMSReply(GSMMessageType::full, id);
            }
            else
            {
                _learning = true;
                // SMS: send learning ok
                sendTypeMessage(LCDMessageType::status, literals::simplyOK, false);
                sendSMSReply(GSMMessageType::add, id);
            }
            break;
        }

        if (cmd == GSMMessageType::none)
        {
            // unknown command - send help
            sendTypeMessage(LCDMessageType::status, literals::simplyOK, false);
            sendSMSReply(GSMMessageType::none, id);
            break;
        }

        // output state
        if (cmd == GSMMessageType::state) 
        {
            sendSMSReply(GSMMessageType::state, id);
            // send list of all users
            // SMS: list of users
            break;
        }

        if (cmd == GSMMessageType::list)
        {
            sendTypeMessage(LCDMessageType::status, literals::simplyOK, false);
            sendSMSReply(GSMMessageType::list, id);
            // send list of all users
            // SMS: list of users
            break;
        }

        // special command - all off
        if (cmd == GSMMessageType::alloff)
        {
            sendTypeMessage(LCDMessageType::status, literals::simplyOK, false);

            OutputMsg msgx;
            msgx._output = pin;
            msgx._value = onOff;
            msgx._messageType = OutputTypeMsg::writeAllOff;
            Application::getInstance()->getOutputTask()->message(msgx, false);
            sendSMSReply(GSMMessageType::accepted, id);
        }

        // commands - output on/off
        if ((cmd != GSMMessageType::none) && (pin != UINT32_MAX))
        {

            sendTypeMessage(LCDMessageType::status, literals::simplyOK, false);

            OutputMsg msgx;
            msgx._output = pin;
            msgx._value = onOff;
            msgx._messageType = OutputTypeMsg::writeone;
            Application::getInstance()->getOutputTask()->message(msgx, false);
            sendSMSReply(GSMMessageType::accepted, id);
        }

    } while (false);

    // delete last sms
    _gsm.delSMS(_gsm.incomingSMSIndex());

    startView();
}

// -------------------------------------------------------------------------------------------------

bool GSMTask::processGSMStatus()
{
    bool rc = true;
    auto stx = _gsm.checkStatus();

    if (stx == gsm::ResponseStatus::callerid)
    {
        auto callerID = _gsm.getCallersID();
        // always hang
        _gsm.disconnect();

        if (true /*_learning*/)
        {
            ringOperation(callerID.c_str());
        }
    }

    if (stx == gsm::ResponseStatus::ring)
    {
        sendTypeMessage(LCDMessageType::backlon, literals::empty, false);
        sendTypeMessage(LCDMessageType::idcaller, literals::ring, false);
    }

    if (stx == gsm::ResponseStatus::newsms)
    {
        sendTypeMessage(LCDMessageType::backlon, literals::empty, false);
        auto xsms = _gsm.readSMS(_gsm.incomingSMSIndex());
        if (xsms.has_value())
        {
            auto [msg, id, tmx] = xsms.value();
            smsOperation(msg, id, tmx);
        }
    } else {
        rc = false;
    }

    /*
    if (stx == gsm::ResponseStatus::nodial)
    {
        sendTypeMessage(LCDMessageType::idcaller, "nodial", false);
    }

    if (stx == gsm::ResponseStatus::busy)
    {
        sendTypeMessage(LCDMessageType::idcaller, "busy", false);
    }
    */

   return rc;
}

// -------------------------------------------------------------------------------------------------

bool GSMTask::init(const char *name, UBaseType_t priority, const configSTACK_DEPTH_TYPE stackDepth)
{
    return RPTask::init(name, priority, stackDepth);
}

// -------------------------------------------------------------------------------------------------