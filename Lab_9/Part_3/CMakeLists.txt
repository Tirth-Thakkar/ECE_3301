cmake_minimum_required(VERSION 3.21.1)
set(CMAKE_SYSTEM_NAME "Generic")

project(Lab_9_Part_3 LANGUAGES C ASM)
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
add_library(Altimeter OBJECT src/Altimeter.c)
add_library(MotionSensor OBJECT src/MotionSensor.c)
add_library(LIS3DH OBJECT src/LIS3DH.c)
add_library(Button OBJECT ../Common/src/Button.c)
add_library(Indicators OBJECT ../Common/src/Indicators.c)
add_library(LCD OBJECT ../Common/src/LCD.c)

# Keep the working Lab 8 bus and pressure-sensor drivers.
add_library(BMP280 OBJECT ../../Lab_8/src/BMP280.c)
add_library(I2C OBJECT ../../Lab_8/src/I2C.c)

add_executable(Lab9_Part3_app src/main.c)
target_sources(Lab9_Part3_app PRIVATE
    $<TARGET_OBJECTS:ADC>
    $<TARGET_OBJECTS:Dashboard>
    $<TARGET_OBJECTS:StateController>
    $<TARGET_OBJECTS:System>
    $<TARGET_OBJECTS:Altimeter>
    $<TARGET_OBJECTS:MotionSensor>
    $<TARGET_OBJECTS:LIS3DH>
    $<TARGET_OBJECTS:BMP280>
    $<TARGET_OBJECTS:I2C>
    $<TARGET_OBJECTS:Button>
    $<TARGET_OBJECTS:Indicators>
    $<TARGET_OBJECTS:LCD>
)
