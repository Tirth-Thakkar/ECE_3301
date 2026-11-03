cmake_minimum_required(VERSION 3.21)
project(Lab_8 LANGUAGES C ASM)

# Invoke with -DCMAKE_TOOLCHAIN_FILE=cmake/PIC18F46K22.cmake
# PIC18F46K22, 16 MHz INTOSC; the macro MUST reach both main.c and lcd.c.
add_executable(Lab8_app
    src/main.c
    src/lcd.c
)
target_include_directories(Lab8_app PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(Lab8_app PRIVATE _XTAL_FREQ=16000000UL)
# The selected XC8 C99 mode uses IEEE 754 32-bit float and double.
# Verify the XC8 project properties (float/double = 32 bits) before programming.
set_target_properties(Lab8_app PROPERTIES SUFFIX ".elf")
