cmake_minimum_required(VERSION 3.21.1)
set(CMAKE_SYSTEM_NAME "Generic")

project(Lab_1 LANGUAGES C ASM)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(MCU_FLAGS "-mcpu=${MCU}")
add_compile_options(${MCU_FLAGS})
add_link_options(${MCU_FLAGS})

# Define the build target
add_executable(Lab1_app 
    src/main.c
)

target_include_directories(Lab1_app 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
