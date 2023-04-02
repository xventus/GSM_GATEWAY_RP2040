/*
 * FreeRTOS V202107.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 * 
 * minor modifications from the original and based on 
 * https://github.com/FreeRTOS/FreeRTOS-SMP-Demos/blob/main/FreeRTOS/Demo/CORTEX_M0+_RP2040/README.md
 * 
 */


#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

// Scheduler Related 

#define configUSE_PREEMPTION                    1       //  Set to 1 to use the preemptive RTOS scheduler, or 0 to use the cooperative RTOS scheduler.
#define configUSE_TICKLESS_IDLE                 0       // 1 to use the low power tickless mode, or 0 to keep the tick interrupt running at all times
#define configUSE_IDLE_HOOK                     0       // Set to 1 if you wish to use an idle hook, or 0 to omit an idle hook - vApplicationIdleHook
#define configUSE_TICK_HOOK                     0       // Set to 1 if you wish to use an tick hook, or 0 to omit an tick hook.
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )  // The frequency of the RTOS tick interrupt.  1000 for demo

#define configMAX_PRIORITIES                    5      // The number of priorities available to the application tasks.
                                                        // Each available priority consumes a little RAM within the RTOS kernel so this value should not be set any higher 
                                                        // than actually required by your application. Maximum permissible value will be capped if configUSE_PORT_OPTIMISED_TASK_SELECTION is set to 1 

#define configMINIMAL_STACK_SIZE                ( configSTACK_DEPTH_TYPE ) 256 // The size of the stack used by the idle task
#define configUSE_16_BIT_TICKS                  0  // 0 - 0 causes TickType_t to be defined as an unsigned 32bit type
                                                   // Time is measured in 'ticks' - which is the number of times the tick interrupt has executed since the RTOS kernel was started. 

#define configIDLE_SHOULD_YIELD                 1   // if configIDLE_SHOULD_YIELD is set to 1 then the idle task will yield immediately if any other task at the idle priority is ready to run 

// Synchronization Related

#define configUSE_MUTEXES                       1   // Set to 1 to include mutex functionality in the build, or 0 to omit mutex functionality from the build. 
#define configUSE_RECURSIVE_MUTEXES             1   // Set to 1 to include recursive mutex functionality in the build, or 0 to omit recursive mutex functionality from the build.
#define configUSE_APPLICATION_TASK_TAG          1   // [DBG]
#define configUSE_COUNTING_SEMAPHORES           1   // Set to 1 to include counting semaphore functionality in the build, or 0 to omit counting semaphore functionality from the build.
#define configQUEUE_REGISTRY_SIZE               8   // [DBG] !!! The queue registry has no purpose unless you are using a RTOS kernel aware debugger. 
                                                    // Defines the maximum number of queues and semaphores that can be registered. 
#define configUSE_QUEUE_SETS                    1   // Set to 1 to include queue set functionality (the ability to block, or pend, on multiple queues and semaphores), or 0 to omit queue set functionality.
#define configUSE_TIME_SLICING                  1   // 1 - FreeRTOS uses prioritised preemptive scheduling with time slicing, 0- scheduler will still run the highest priority task that is in the Ready state, 
                                                    // but will not switch between tasks of equal priority just because a tick interrupt has occurred
#define configUSE_NEWLIB_REENTRANT              0   // newlib support - not used by the FreeRTOS maintainers themselves !
#define configENABLE_BACKWARD_COMPATIBILITY     0   // configENABLE_BACKWARD_COMPATIBILITY to 0 in FreeRTOSConfig.h excludes the macros from the build, and in so doing allowing validation that no pre version 8.0.0 names are being used
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5   // TLS see https://freertos.org/thread-local-storage-pointers.html

// System 

#define configSTACK_DEPTH_TYPE                  uint32_t    // sets the type used to specify the stack depth in calls to xTaskCreate()
#define configMESSAGE_BUFFER_LENGTH_TYPE        size_t      // configMESSAGE_BUFFER_LENGTH_TYPE to store the length of each message

// Memory allocation related definitions. 

#define configSUPPORT_STATIC_ALLOCATION         0   //  0 -  RTOS objects can only be created using RAM allocated from the FreeRTOS heap
                                                    //  1 - objects can be created using RAM provided by the application writer

#define configSUPPORT_DYNAMIC_ALLOCATION        1   //  1 - objects can be created using RAM that is automatically allocated from the FreeRTOS heap
                                                    //  0 - objects can only be created using RAM provided by the application writer

#define configTOTAL_HEAP_SIZE                   (128*1024)  // The total amount of RAM available in the FreeRTOS heap (configSUPPORT_DYNAMIC_ALLOCATION)
#define configAPPLICATION_ALLOCATED_HEAP        0           // 0 - heap is declared by FreeRTOS and placed in memory by the linker
                                                            // 1 - specific heap see uint8_t ucHeap[ configTOTAL_HEAP_SIZE ]

// Hook function related definitions. 

#define configCHECK_FOR_STACK_OVERFLOW          2   // Stack overflow detection method 1 or 2  - hook vApplicationStackOverflowHook - https://freertos.org/Stacks-and-stack-overflow-checking.html
#define configUSE_MALLOC_FAILED_HOOK            1   // 1 then the application must define a malloc() failed hook -  vApplicationMallocFailedHook
#define configUSE_DAEMON_TASK_STARTUP_HOOK      1   // executing after the scheduler has been started - vApplicationDaemonTaskStartupHook()

// Run time and task stats gathering related definitions. 

#define configGENERATE_RUN_TIME_STATS           1   // 0 [DBG] collect information on the amount of processing time that has been used by each task - vTaskGetRunTimeStats
#define configUSE_TRACE_FACILITY                1   // [DBG] set to 1 if you wish to include additional structure members and functions to assist with execution visualisation and tracing
#define configUSE_STATS_FORMATTING_FUNCTIONS    1   // 0 [DBG] 1 to include the vTaskList() and vTaskGetRunTimeStats() functions in the build


extern unsigned long getTimerForRunTimeStats(); 
extern void  configureTimerForRunTimeStats();
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()    configureTimerForRunTimeStats()  // [DBG] ADDED
#define portGET_RUN_TIME_COUNTER_VALUE()            getTimerForRunTimeStats()  // [DBG] ADDED





// Co-routine related definitions. 

#define configUSE_CO_ROUTINES                   0   // Set to 1 to include co-routine functionality in the build -  croutine.c
#define configMAX_CO_ROUTINE_PRIORITIES         1   // The number of priorities available to the application co-routines. 

// Software timer related definitions. 
#define configUSE_TIMERS                        1 // Set to 1 to include software timer functionality, or 0 to omit software timer functionality
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10      // Sets the length of the software timer command queue
#define configTIMER_TASK_STACK_DEPTH            1024    // Sets the stack depth allocated to the software timer service/daemon task.

// Interrupt nesting behaviour configuration. 

// #define configKERNEL_INTERRUPT_PRIORITY         [dependent of processor]
// #define configMAX_SYSCALL_INTERRUPT_PRIORITY    [dependent on processor and application]
// #define configMAX_API_CALL_INTERRUPT_PRIORITY   [dependent on processor and application]

// SMP port only 

#define configNUM_CORES                         2       //  number of cores, rp2040 - TWO, Note: jlink debug switch to 1, using one core
#define configTICK_CORE                         0       //
#define configRUN_MULTIPLE_PRIORITIES           1       // Enable/Disable simultaneously running tasks with multiple priorities
#define configUSE_CORE_AFFINITY                 1       // Enable/Disable setting a task's affinity to certain cores

// RP2040 specific 
#define configSUPPORT_PICO_SYNC_INTEROP         1
#define configSUPPORT_PICO_TIME_INTEROP         1

#include <assert.h>
// Define to trap errors during development. 
#define configASSERT(x)                         assert(x)

// Set the following definitions to 1 to include the API function, or zero
// to exclude the API function.

#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 1
#define INCLUDE_xTaskGetHandle                  1
#define INCLUDE_xTaskResumeFromISR              1
#define INCLUDE_xQueueGetMutexHolder            1

// A header file that defines trace macro can be included here. 

#endif // FREERTOS_CONFIG_H 



