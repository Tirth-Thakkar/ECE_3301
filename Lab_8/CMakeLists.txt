cmake_minimum_required(VERSION 3.21.1)
set(CMAKE_SYSTEM_NAME "Generic")

project(Lab_5 LANGUAGES C ASM)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(MCU_FLAGS "-mcpu=${MCU}")
add_compile_options(${MCU_FLAGS})
add_link_options(${MCU_FLAGS})

add_library(LCD OBJECT
    src/lcd.c
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
    $<TARGET_OBJECTS:LCD>
)
