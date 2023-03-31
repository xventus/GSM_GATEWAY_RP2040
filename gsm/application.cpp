//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   application.h
/// @author Petr Vanek

#include "application.h"
#include "src-utils/debug_utils.h"
#include "hardware.h"

// global application instance as singleton and instance acquisition.
// Application gApp;
// Application *APPLICATION = &gApp;

Application *Application::getInstance()
{
    static Application instance;
    return &instance;
}

// const Application* APPLICATION = Application::getInstance();

Application::Application() : _heartBeat(PIN_HEART_BEAT)
{
}

Application::~Application()
{
    done();
}

void Application::uartRxIRQHandle()
{

    while (uart_is_readable(TERMINAL_UART_ID))
    {

        auto ch = uart_getc(TERMINAL_UART_ID);
        //printf("%c",ch);
        Application::getInstance()->getTerminalTask()->message(ch, true);
    }
}

void Application::runtimeStatistic()
{
    char buff[2048] = {0};
    vTaskGetRunTimeStats(buff);
    dbgPrint(literals::separator);
    dbgPrint(literals::header);
    dbgPrint(literals::separator);
    dbgPrint(buff);
    dbgPrint(literals::separator);
}

bool Application::irqHandlersInit()
{
    bool rc = false;

    do
    {

        // GPS serial IRQ
        uart_init(TERMINAL_UART_ID, TERMINAL_BAUD_RATE);
        gpio_set_function(TERMINAL_UART_TX_PIN, GPIO_FUNC_UART);
        gpio_set_function(TERMINAL_UART_RX_PIN, GPIO_FUNC_UART);
        uart_set_fifo_enabled(TERMINAL_UART_ID, false);
        irq_set_exclusive_handler(UART1_IRQ, &uartRxIRQHandle);
        irq_set_enabled(UART1_IRQ, true);
        uart_set_irq_enables(TERMINAL_UART_ID, true, false);

        rc = true;

    } while (false);

    return rc;
}

void Application::init()
{

    stdio_uart_init_full(uart1, TERMINAL_BAUD_RATE, TERMINAL_UART_TX_PIN, TERMINAL_UART_RX_PIN);

    dbgLog("init");

    do
    {
        uint8_t components = 0;
        if (!irqHandlersInit())
            break;
        components++;
        
       if (!_heartBeat.init(literals::tsk_led, tskIDLE_PRIORITY + 1ul, configMINIMAL_STACK_SIZE))
            break;
        components++;


        if (!_outputs.init(literals::tsk_oututs, tskIDLE_PRIORITY + 1ul, 1024 /*configMINIMAL_STACK_SIZE*/))
            break;
        components++;

        if (!_lcd.init(literals::tsk_lcd, tskIDLE_PRIORITY + 1ul, 1024 /*configMINIMAL_STACK_SIZE*/))
            break;
        components++;

        if (!_gsm.init(literals::tsk_gsm, tskIDLE_PRIORITY + 1ul, 1024 /*configMINIMAL_STACK_SIZE*/))
            break;
        components++;

        if (!_tick.init())
            break;
        components++;

        if (!_terminal.init(literals::tsk_term, tskIDLE_PRIORITY + 1ul, 1024))
            break;
        components++;

        dbgLog("start scheduler components %d", components);
        // start RTOS scheduler
        vTaskStartScheduler();

    } while (false);
    // if the run gets this far, the allocation has obviously failed
}

void Application::done()
{
    // fail indication - complete initialization to be sure
    gpio_init(PIN_HEART_BEAT);
    gpio_set_dir(PIN_HEART_BEAT, GPIO_OUT);
    gpio_put(PIN_HEART_BEAT, 1);
}

Application *Application::operator->()
{
    return Application::getInstance();
}

Application *const Application::operator->() const
{
    return Application::getInstance();
}
