
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   lcd_task.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "output_task.h"
#include "pico/stdlib.h"
#include "src-utils/debug_utils.h"
#include "literals.h"
#include "gsm_message.h"
#include "application.h"

OutputTask::OutputTask()
{
    _queueRequest = xQueueCreate(5, sizeof(OutputMsg));
}

OutputTask::~OutputTask()
{
    done();
    if (_queueRequest)
        vQueueDelete(_queueRequest);
}

uint32_t OutputTask::getOutputmask()
{

    uint32_t mask = 0;

    for (const auto &x : gsmCommandArray)
    {
        if (x._pin != UINT32_MAX)
        {
            mask |= (1 << x._pin);
        }
    }

    return mask;
}


uint32_t OutputTask::outputsToOrder(const uint32_t outputs) 
{
    uint32_t rc = 0;
    uint32_t cntx = 0;
    for (const auto &x : gsmCommandArray)
    {
        // only valid output pins definition
        if (x._pin == UINT32_MAX)
            continue;

        if (x._onOff == false)
            continue;

        // marking of individual output pins sequentially with a number
        auto pinposition = (1 << x._pin);
        if (outputs & pinposition)
        {
            rc |= (1 << cntx);
        }

        cntx++; 
    }
    return rc;
}


std::string OutputTask::outputsToString(const uint32_t outputs) 
{
    uint32_t cntx = 0;
    std::string tmp;
    for (const auto &x : gsmCommandArray)
    {
        // only valid output pins definition
        if (x._pin == UINT32_MAX)
            continue;

        if (x._onOff == false)
            continue;

        // marking of individual output pins sequentially with a number
        cntx++;

        auto pinposition = (1 << x._pin);
        if (outputs & pinposition)
        {
            tmp += std::to_string(cntx);
        }
        else
        {
            tmp += "-";
        }
    }
    return tmp;
}

void OutputTask::loop()
{
    uint32_t outputs = 0; // image of outputs - real pin position
    LCDMessage lcdmsg;
    GSMMessage gsmmsg;
    std::string tmp;
    uint32_t cntx = 0;
    TerminalMessage trmmsg;

    auto mask = getOutputmask();
    gpio_init_mask(mask);
    gpio_set_dir_out_masked(mask);
    gpio_clr_mask(mask); // all pins OFF

    while (true)
    { // Loop forever
        OutputMsg msg;
        auto res = xQueueReceive(_queueRequest, (void *)&msg, (TickType_t)50 / portTICK_PERIOD_MS);
        if (res == pdTRUE)
        {
            switch (msg._messageType)
            {

            case OutputTypeMsg::writeAllOff:

                outputs = 0;
                gpio_clr_mask(mask); // all pins OFF

                break;

            case OutputTypeMsg::writeone:

                if (msg._value)
                {
                    gpio_set_mask(1ul << (msg._output));
                    outputs |= (1ul << msg._output);
                }
                else
                {
                    gpio_clr_mask(1ul << (msg._output));
                    outputs &= ~(1ul << msg._output);
                }

                break;

            case OutputTypeMsg::readall:

                lcdmsg._messageType = LCDMessageType::idcaller;
                lcdmsg._value = outputs;
                memset(&lcdmsg._message, 0, sizeof(lcdmsg._message));
                cntx = 0;
                tmp.clear();
                tmp += literals::output;
                tmp += outputsToString(outputs);
                strncpy(lcdmsg._message, tmp.c_str(), sizeof(lcdmsg._message) - 1);
                Application::getInstance()->getLCDTask()->message(lcdmsg, false);

                memset(gsmmsg._message, 0, sizeof(gsmmsg._message));
                strncpy(gsmmsg._message, tmp.c_str(), sizeof(lcdmsg._message) - 1);
                gsmmsg._messageType = GSMMessageType::state;
                Application::getInstance()->getGSMTask()->message(gsmmsg, false);
                break;

            case OutputTypeMsg::readallTerm:
                trmmsg._messageType = TerminalMessageType::readAllAck;
                trmmsg._value = outputsToOrder(outputs);
                Application::getInstance()->getTerminalTask()->message(trmmsg, false);
                break;

            case OutputTypeMsg::writeAllOffTerm:

                gpio_clr_mask(mask); // all pins OFF
                outputs = 0;
                TerminalMessage trmmsg;
                trmmsg._messageType = TerminalMessageType::clearAllAck;
                trmmsg._value = outputs;
                Application::getInstance()->getTerminalTask()->message(trmmsg, false);
                break;
            }
        }
    }
}

bool OutputTask::init(const char *name, UBaseType_t priority, const configSTACK_DEPTH_TYPE stackDepth)
{
    return RPTask::init(name, priority, stackDepth);
}

void OutputTask::writeToOutput(uint8_t outputId, bool on, bool isr)
{
    OutputMsg msg;
    msg._value = on;
    msg._output = outputId;
    msg._messageType = OutputTypeMsg::writeone;
    if (_queueRequest)
    {
        (isr) ? xQueueSendToBackFromISR(_queueRequest, (void *)&msg, 0) : xQueueSendToBack(_queueRequest, (void *)&msg, 0);
    }
}

void OutputTask::message(const OutputMsg &msg, bool isr)
{
    if (_queueRequest)
    {
        (isr) ? xQueueSendToBackFromISR(_queueRequest, (void *)&msg, 0) : xQueueSendToBack(_queueRequest, (void *)&msg, 0);
    }
}
