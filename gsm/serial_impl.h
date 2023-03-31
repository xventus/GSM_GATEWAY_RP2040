//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   literals.h
/// @author Petr Vanek

#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "src-gsm/gsm.h"
#include "hardware.h"

namespace gsm {

/**
 * @brief
 *
 */
class SerialImpl : public ISerialModem
{
public:
    bool isReadable() override
    {
        return uart_is_readable(GSM_UART_ID0); 
    }

    bool isWritable() override
    {
        return uart_is_writable(GSM_UART_ID0);
    }
    void putch(char c) override
    {
        //printf(">%c<",c);
        uart_putc_raw(GSM_UART_ID0, c);
    }
    char getc() override
    {
        char c  = uart_getc(GSM_UART_ID0);
        //printf("%c",c);
        return c;
    }

    void delay(uint16_t ms) override
    {
        vTaskDelay(ms / portTICK_PERIOD_MS);    // RTOS
        //sleep_ms(ms);                         // pico SDK
    }

    uint64_t getus() {
        return time_us_64();
    }

    void hardwareInit() override
    {
        gpio_init(POWER_ENABLE);
        gpio_set_dir(POWER_ENABLE, GPIO_OUT);
        gpio_put(POWER_ENABLE, 1);

        uart_init(GSM_UART_ID0, GSM_BAUD_RATE);
        gpio_set_function(GSM_UART_TX_PIN0, GPIO_FUNC_UART);
        gpio_set_function(GSM_UART_RX_PIN0, GPIO_FUNC_UART);

        uart_set_fifo_enabled(GSM_UART_ID0, true);
    }

    void modemInit() override
    {
        gpio_put(POWER_ENABLE,1);
        delay(500);
        gpio_put(POWER_ENABLE,0);
        delay(500);
        gpio_put(POWER_ENABLE,1);
        delay(2000);
    
    }
};

} //namespace gsm 