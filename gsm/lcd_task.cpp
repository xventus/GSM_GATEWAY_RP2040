
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
#include "lcd_task.h"
#include "pico/stdlib.h"
#include "src-utils/debug_utils.h"
#include "src-lcd5110/generated/codeSquaredRegular.h"
#include "src-lcd5110/generated/modeseven.h"
#include "src-lcd5110/generated/topaz.h"
#include "literals.h"

LCDTask::LCDTask()
{
    _queueDisplay = xQueueCreate(5, sizeof(LCDMessage));
}

LCDTask::~LCDTask()
{
    done();
    if (_queueDisplay)
        vQueueDelete(_queueDisplay);
}

void LCDTask::loop()
{
    // lcd initialization
    uint8_t rowposition = 0;
    Lcd5110 lcd(LCD_SPI, LCD_RST_PIN, LCD_CE_PIN, LCD_DC_PIN, LCD_DIN_PIN, LCD_CLK_PIN, LCD_LIGHT_PIN);

    auto signal = [&lcd](int32_t value)
    {
        lcd.fillRect(0, Lcd5110::maxY - 5, Lcd5110::maxX, 5, true);
        auto incr = (Lcd5110::maxX / 10);
        value = value * 2;
        if (value > 100)
            value = Lcd5110::maxX;

        // 0 - 99
        for (int i = 0; i < Lcd5110::maxX; i += incr)
        {
            lcd.fillRect(i, Lcd5110::maxY - 5, incr - 2, 5);
            if (i >= value)
                break;
        }
    };

    lcd.init(LCD_BIAS, LCD_CONTRAST);
    lcd.backLight(true);
    lcd.cls();
    lcd.refresh();

    while (true)
    { // Loop forever
        LCDMessage msg;
        auto res = xQueueReceive(_queueDisplay, (void *)&msg, (TickType_t)50 / portTICK_PERIOD_MS);
        if (res == pdTRUE)
        {
            switch (msg._messageType)
            {

            case LCDMessageType::backlon:
                lcd.backLight(true);
                break;

            case LCDMessageType::backloff:
                lcd.backLight(false);
                break;

            case LCDMessageType::init:
                lcd.cls();
                lcd.refresh();
                break;

            case LCDMessageType::idcaller:
                lcd.withSize(1);
                lcd.withFont(&topaz::fnt[0][0], topaz::glypHeight, topaz::glypWidth, topaz::firstChar, topaz::glyps);
                lcd.at(0, 30).print(msg._message);
                lcd.refresh();
                break;

            case LCDMessageType::provider:
                lcd.withSize(1);
                lcd.withFont(&topaz::fnt[0][0], topaz::glypHeight, topaz::glypWidth, topaz::firstChar, topaz::glyps);
                lcd.at(0, 0).print(msg._message);
                lcd.refresh();
                break;

            case LCDMessageType::signal:
                signal(msg._value);
                lcd.refresh();
                break;

            case LCDMessageType::date:
                lcd.withSize(1);
                lcd.withFont(&topaz::fnt[0][0], topaz::glypHeight, topaz::glypWidth, topaz::firstChar, topaz::glyps);
                lcd.at(0, 10).print(msg._message);
                lcd.refresh();
                break;

            case LCDMessageType::time:
                lcd.withSize(1);
                lcd.withFont(&topaz::fnt[0][0], topaz::glypHeight, topaz::glypWidth, topaz::firstChar, topaz::glyps);
                lcd.at(0, 20).print(msg._message);
                lcd.refresh();
                break;

            case LCDMessageType::number:
                if (msg._value == 0 || msg._value == 1)
                {
                    lcd.withSize(1);
                    lcd.cls();
                    lcd.withFont(&topaz::fnt[0][0], topaz::glypHeight, topaz::glypWidth, topaz::firstChar, topaz::glyps);
                    lcd.at(0, 0).print(literals::welcome);
                }

                lcd.fillRect(0, 10, Lcd5110::maxX, Lcd5110::maxY - 10, true);
                lcd.withSize(2);
                lcd.withFont(&codeSquaredRegular::fnt[0][0], codeSquaredRegular::glypHeight, codeSquaredRegular::glypWidth, codeSquaredRegular::firstChar, codeSquaredRegular::glyps);
                if (msg._value < 10)
                    lcd.at(35, 20);
                else
                    lcd.at(25, 20);
                lcd.print(msg._value);
                signal(msg._value * 2);
                lcd.refresh();
                break;

            case LCDMessageType::status:
                if (msg._value == 0)
                {
                    rowposition = 0;
                    lcd.withSize(1);
                    lcd.cls();
                    lcd.withFont(&topaz::fnt[0][0], topaz::glypHeight, topaz::glypWidth, topaz::firstChar, topaz::glyps);
                    lcd.at(0, 0).print(literals::welcome);
                }

                lcd.withFont(&modeseven::fnt[0][0], modeseven::glypHeight, modeseven::glypWidth, modeseven::firstChar, modeseven::glyps);

                if (rowposition > 4)
                {
                    lcd.fillRect(0, 10, Lcd5110::maxX, Lcd5110::maxY, true);
                    rowposition = 0;
                }

                lcd.at(0, 10 + (rowposition * 10));
                lcd.print(msg._message);
                lcd.refresh();
                rowposition++;

                break;
            }
        }
    }
}

bool LCDTask::init(const char *name, UBaseType_t priority, const configSTACK_DEPTH_TYPE stackDepth)
{
    return RPTask::init(name, priority, stackDepth);
}

void LCDTask::message(const LCDMessage &msg, bool isr)
{
    if (_queueDisplay)
    {
        (isr) ? xQueueSendToBackFromISR(_queueDisplay, (void *)&msg, 0) : xQueueSendToBack(_queueDisplay, (void *)&msg, 0);
    }
}