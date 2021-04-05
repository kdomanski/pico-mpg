cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(mpg main.cpp display.cpp uart.cpp)

pico_enable_stdio_usb(mpg 1)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(mpg pico_stdlib hardware_uart hardware_spi)

# create map/bin/hex file etc.
pico_add_extra_outputs(mpg)
