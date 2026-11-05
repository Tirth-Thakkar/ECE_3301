cmake_minimum_required(VERSION 3.21.1)
set(CMAKE_SYSTEM_NAME "Generic")

project(Lab_8 LANGUAGES C ASM)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(MCU_FLAGS "-mcpu=${MCU}")
add_compile_options(${MCU_FLAGS})
add_link_options(${MCU_FLAGS})

add_library(BMP280 OBJECT
    src/BMP280.c
)

add_library(Dashboard OBJECT
    src/Dashboard.c
)

add_library(I2C OBJECT
    src/I2C.c
)

add_library(LCD OBJECT
    src/LCD.c
)

# Define the build target
add_executable(Lab8_app 
    src/main.c
)

target_include_directories(Lab8_app 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(Lab8_app
    PRIVATE
    $<TARGET_OBJECTS:BMP280>
    $<TARGET_OBJECTS:Dashboard>
    $<TARGET_OBJECTS:I2C>
    $<TARGET_OBJECTS:LCD>
)
