cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(uhr_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(uhr uhr.c )

# Bibliotheksverzeichnisse
add_subdirectory(./ili_9341)
add_subdirectory(./si4703)
add_subdirectory(./buttons)
add_subdirectory(./momefilo_flash)

target_link_libraries(uhr
	ili_9341
	buttons
	pico_stdlib
	pico_double
	pico_printf
	pico_malloc
	pico_util
	si4703
	momefilo_flash
	hardware_i2c
	hardware_rtc)
pico_add_extra_outputs(uhr)
