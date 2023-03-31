# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Include RTOS kernel
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# add PICO EXTRAS
include( $ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake )

include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

# Set name of project (as PROJECT_NAME) and C/C%2B%2B standards
project(GSM_GATEWAY C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME} 
    rtos_hooks.cpp
    main.cpp
    led_task.cpp
    application.cpp
    rptask.cpp
    rptimer.cpp
    gsm_tick.cpp
    output_task.cpp
    lcd_task.cpp
    gsm_task.cpp
    commanders.cpp
    terminal_task.cpp
    sms_command_analyzer.cpp
    src-gsm/gsm.cpp
    src-gsm/at_parser.cpp
    src-utils/time_base.cpp
    src-lcd5110/lcd5110.cpp
)

target_include_directories(${PROJECT_NAME}  
PRIVATE ${CMAKE_CURRENT_LIST_DIR}
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    hardware_i2c
    hardware_pwm
    hardware_pio
    hardware_spi
    hardware_dma
    hardware_rtc
    hardware_adc
    hardware_flash
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND arm-none-eabi-size ${CMAKE_CURRENT_LIST_DIR}/build/${PROJECT_NAME}.elf
    VERBATIM
)

#set(PICO_DEOPTIMIZED_DEBUG=1)
