cmake_minimum_required(VERSION 3.21.1)
set(CMAKE_SYSTEM_NAME "Generic")

project(Lab_9_Part_3 LANGUAGES C ASM)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(MCU_FLAGS "-mcpu=${MCU}")
add_compile_options(${MCU_FLAGS})
add_link_options(${MCU_FLAGS})
add_compile_definitions(_XTAL_FREQ=16000000UL)

add_library(ADC OBJECT
    src/ADC.c
)

add_library(Altimeter OBJECT
    src/Altimeter.c
)

add_library(BMP280 OBJECT
    src/BMP280.c
)

add_library(Dashboard OBJECT
    src/Dashboard.c
)

add_library(I2C OBJECT
    src/I2C.c
)

add_library(ICM20948 OBJECT
    src/ICM20948.c
)

add_library(MotionSensor OBJECT
    src/MotionSensor.c
)

add_library(StateController OBJECT
    src/StateController.c
)

add_library(System OBJECT
    src/System.c
)

add_library(Button OBJECT
    ../lib/src/Button.c
)

add_library(Indicators OBJECT
    ../lib/src/Indicators.c
)

add_library(LCD OBJECT
    ../lib/src/LCD.c
)

add_executable(Lab9_Part3_app
    src/main.c
)

target_include_directories(Lab9_Part3_app
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include
)

target_sources(Lab9_Part3_app PRIVATE
    $<TARGET_OBJECTS:ADC>
    $<TARGET_OBJECTS:Dashboard>
    $<TARGET_OBJECTS:StateController>
    $<TARGET_OBJECTS:System>
    $<TARGET_OBJECTS:Altimeter>
    $<TARGET_OBJECTS:MotionSensor>
    $<TARGET_OBJECTS:ICM20948>
    $<TARGET_OBJECTS:BMP280>
    $<TARGET_OBJECTS:I2C>
    $<TARGET_OBJECTS:Button>
    $<TARGET_OBJECTS:Indicators>
    $<TARGET_OBJECTS:LCD>
)
