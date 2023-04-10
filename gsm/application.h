//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   application.h
/// @author Petr Vanek

#pragma once

#include <FreeRTOS.h>
#include <memory>
#include <atomic>
#include "pico/stdlib.h"
#include <stdio.h>
#include "literals.h"
#include "led_task.h"
#include "lcd_task.h"
#include "gsm_task.h"
#include "terminal_task.h"
#include "output_task.h"
#include "gsm_tick.h"

/**
 * @brief GSM gateway application class - singleton
 *
 */
class Application
{

public:
    
    /**
     * @brief Destroy the Application object
     *
     */
    virtual ~Application();

    /**
     * @brief application initialization
     *
     */
    void init();

    /**
     * @brief application deinitialization, an error occurred
     *
     */
    void done();

    /**
     * @brief Get the Output Task object access
     *
     * @return OutputTask*
     */
    OutputTask *getOutputTask() { return &_outputs; }

    /**
     * @brief Get heart beat task access
     *
     * @return LedTask*
     */
    LedTask *getLEDTask() { return &_heartBeat; }

    /**
     * @brief Get LCD view task access
     *
     * @return LCDTask*
     */
    LCDTask *getLCDTask() { return &_lcd; }

    /**
     * @brief Get GSM&GPS access taks
     *
     * @return GSMTask*
     */
    GSMTask *getGSMTask() { return &_gsm; }

    /**
     * @brief Get the Terminal Task
     *
     * @return TerminalTask*
     */
    TerminalTask *getTerminalTask() { return &_terminal; }

    /**
     * @brief Get ticker task - used for refresh status information from GSM modem
     *
     * @return GSMTick*
     */
    GSMTick *getGSMTick() { return &_tick; }

    /**
     * Singleton
    */
    Application *operator->();
    Application *const operator->() const;
    static Application *getInstance();
    static void runtimeStatistic();

protected:
    /**
     * @brief The button has been pressed
     *
     * @param gpio - GPIO pin
     * @param events  - event
     */
    static void keyboardIRQHandler(uint gpio, uint32_t events);

    /**
     * @brief ISR for receiving chat from GPS receiver
     *
     */
    static void uartRxIRQHandle();

private:
    /**
     * @brief Initialization of IRQ handler and related pins
     *
     * @return true - success
     * @return false
     */
    bool irqHandlersInit();

    /**
     * @brief Constructor
     *
     */
    Application();
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;


    LedTask _heartBeat;         ///< led task instance
    LCDTask _lcd;               ///< lcd task instance
    GSMTask _gsm;               ///< gsm task instance
    TerminalTask _terminal;     ///< terminal task instance
    GSMTick _tick;              ///< tick task instance
    OutputTask _outputs;        ///< output control task instance
};
