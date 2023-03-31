//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   hardware.h
/// @author Petr Vanek

#pragma once

#include "hardware/uart.h"

/**
 * @brief Hardware definition, pin assignment, communication interfaces settings
 * 
 */


// misc
#define PIN_HEART_BEAT PICO_DEFAULT_LED_PIN

/**
 * @brief setting parameters for serial modem - GSM + GNSS
 *
 */
#define GSM_UART_ID0 uart0
#define GSM_BAUD_RATE 115200
#define GSM_DATA_BITS 8
#define GSM_STOP_BITS 1
#define GSM_PARITY UART_PARITY_NONE
#define GSM_UART_TX_PIN0 0
#define GSM_UART_RX_PIN0 1
#define POWER_ENABLE 14 // Pull down to shutdown

/**
 * @brief Terminal UART - communication via RS485 - half duplex
 * 
 */
#define TERMINAL_UART_ID uart1
#define TERMINAL_BAUD_RATE 115200
#define TERMINAL_UART_TX_PIN 4
#define TERMINAL_UART_RX_PIN 5

/**
 * @brief LCD view, LCD 5110 graphical display
 * 
 */
#define LCD_SPI   spi0
#define LCD_BIAS   3 
#define LCD_CONTRAST 70
#define LCD_RST_PIN 8
#define LCD_CE_PIN 12
#define LCD_DC_PIN 11
#define LCD_DIN_PIN 7
#define LCD_CLK_PIN 6
#define LCD_LIGHT_PIN 10

// GSM gateway reinitialization, press after reset clears all commanders
#define REINIT_BUTTON_PIN 21

// AUX output pins GPIO 16 -22 
#define AUX1_PIN     16
#define AUX2_PIN     17
#define AUX3_PIN     18
#define AUX4_PIN     19
#define AUX5_PIN     20
#define AUX6_PIN     21
#define AUX7_PIN     22

#define  HEART_BEAT_LED  PICO_DEFAULT_LED_PIN


