file(GLOB FILES *.c *.h)
add_library(ili_9341 ${FILES})
target_link_libraries(ili_9341
	hardware_spi
	hardware_gpio
	pico_stdlib
)

target_include_directories(ili_9341 PUBLIC ./)
