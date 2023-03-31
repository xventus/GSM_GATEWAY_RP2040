
//
// vim: ts=4 et
// Copyright (c) 2022 Petr Vanek, petr@fotoventus.cz
//
/// @file   rtos_hooks.cpp
/// @author Petr Vanek
//
// @brief - Hooks for RTOS.  
// Depending on the settings in the FreeRTOSConfig.h configuration, they can be omitted or used for catching different states. 

#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "src-utils/debug_utils.h"

/**
 * @brief when the kernel calls the pvPortMalloc allocation function and it fails then this hook is called
 * Note: configTOTAL_HEAP_SIZE, configUSE_MALLOC_FAILED_HOOK
 */
extern "C" void vApplicationMallocFailedHook()
{
    // Force an assert
    configASSERT( ( volatile void * ) NULL );

    dbgLog("vApplicationMallocFailedHook");
}


/**
 * @brief Stack overflof detection method 1 or 2 hooked
 * Note: https://freertos.org/Stacks-and-stack-overflow-checking.html
 * 
 */
extern "C"  void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    // Force an assert
    configASSERT( ( volatile void * ) NULL );
    
    dbgLog("vApplicationStackOverflowHook");
}

/**
 * @brief The idle task runs at the very lowest priority, 
 *  so such an idle hook function will only get executed when there are no tasks of higher priority that are able to run
 * Note: configUSE_IDLE_HOOK
 * 
 */
extern "C"  void vApplicationIdleHook()
{
    /*
    volatile size_t xFreeHeapSpace;
    xFreeHeapSpace = xPortGetFreeHeapSize();

   
    ( void ) xFreeHeapSpace;
    */

     dbgLog("vApplicationIdleHook");
}

/**
 * @brief The tick hook will only get called if configUSE_TICK_HOOK is set to 1
 * Note: vApplicationTickHook() executes from within an ISR so must be very short
 * Note: configUSE_DAEMON_TASK_STARTUP_HOOK
 * 
 */
extern "C" void vApplicationTickHook() 
{
    dbgLog("vApplicationTickHook");
}

/**
 * @brief This is useful if the application includes initialisation code 
 * that would benefit from executing after the scheduler has been started.
 * Note: configUSE_DAEMON_TASK_STARTUP_HOOK
 * 
 */
extern "C" void vApplicationDaemonTaskStartupHook()
{
    dbgLog("vApplicationDaemonTaskStartupHook");
}


uint64_t gRTOSRunTimeTicks;



extern "C"  void configureTimerForRunTimeStats() {
    gRTOSRunTimeTicks = time_us_64();
}

extern "C"  unsigned long getTimerForRunTimeStats() {

    return (time_us_64() - gRTOSRunTimeTicks) / 100;;
}


