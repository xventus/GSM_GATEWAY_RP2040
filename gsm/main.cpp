//
// vim: ts=4 et
// Copyright (c) 2022 Petr Vanek, petr@fotoventus.cz
//
/// @file   main.cpp
/// @author Petr Vanek
//
// @brief - tower clock main

#include "pico/stdlib.h"
#include <stdio.h>
#include "application.h"
#include "src-utils/debug_utils.h"
#include "src-utils/flash_storage.h"

/**
 * @brief reinitialization of GTW for a new installation
 *
 * @return true - if new installation required
 * @return false
 */
bool newInstallationRequired()
{
    bool rc = false;
    gpio_init(REINIT_BUTTON_PIN);
    gpio_set_dir(REINIT_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(REINIT_BUTTON_PIN);
    sleep_ms(200);
    if (!gpio_get(REINIT_BUTTON_PIN))
    {
        rc = true;
        // mark memory as dirty
        FlashStorage fs;
        fs.unusedCommit();
    }
    return rc;
}

/**
 * @brief entry point
 *
 * @return int
 */
int main()
{
    // determine if a new installation is required
    newInstallationRequired();

    
      // check configuration
      // stdio_uart_init_full(uart1, TERMINAL_BAUD_RATE, TERMINAL_UART_TX_PIN, TERMINAL_UART_RX_PIN);
      // FlashStorage fs;
      // fs.memedump();
     

    // start application
    Application::getInstance()->init();
    Application::getInstance()->done();

    // if the RTOS fails
    while (true)
    {
    };
}
