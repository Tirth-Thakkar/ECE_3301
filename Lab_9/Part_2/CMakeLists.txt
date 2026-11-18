cmake_minimum_required(VERSION 3.21.1)
set(CMAKE_SYSTEM_NAME "Generic")

project(Lab_9_Part_2 LANGUAGES C ASM)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(MCU_FLAGS "-mcpu=${MCU}")
add_compile_options(${MCU_FLAGS})
add_link_options(${MCU_FLAGS})
add_compile_definitions(_XTAL_FREQ=16000000UL)

# Every source in this project has one purpose; no section-selection flags.
add_library(ADC OBJECT src/ADC.c)
add_library(Dashboard OBJECT src/Dashboard.c)
add_library(StateController OBJECT src/StateController.c)
add_library(System OBJECT src/System.c)
add_library(Button OBJECT ../lib/src/Button.c)
add_library(Indicators OBJECT ../lib/src/Indicators.c)
add_library(LCD OBJECT ../lib/src/LCD.c)

add_executable(Lab9_Part2_app src/main.c)
target_sources(Lab9_Part2_app PRIVATE
    $<TARGET_OBJECTS:ADC>
    $<TARGET_OBJECTS:Dashboard>
    $<TARGET_OBJECTS:StateController>
    $<TARGET_OBJECTS:System>
    $<TARGET_OBJECTS:Button>
    $<TARGET_OBJECTS:Indicators>
    $<TARGET_OBJECTS:LCD>
)
