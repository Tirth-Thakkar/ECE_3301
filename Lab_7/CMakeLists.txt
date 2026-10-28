cmake_minimum_required(VERSION 3.21.1)
set(CMAKE_SYSTEM_NAME "Generic")

project(Lab_7 LANGUAGES C ASM)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(MCU_FLAGS "-mcpu=${MCU}")
add_compile_options(${MCU_FLAGS})
add_link_options(${MCU_FLAGS})

add_library(StateController OBJECT
    src/StateController.c
)

add_library(LIS3DH OBJECT
    src/LIS3DH.c
)

add_library(SPI OBJECT
    src/SPI.c
)

add_library(UART OBJECT
    src/UART.c
)

# Define the build target
add_executable(Lab7_app 
    src/main.c
)

target_include_directories(Lab7_app 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(Lab7_app
    PRIVATE
    $<TARGET_OBJECTS:LIS3DH>
    $<TARGET_OBJECTS:SPI>
    $<TARGET_OBJECTS:UART>
    $<TARGET_OBJECTS:StateController>
)