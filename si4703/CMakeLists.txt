file(GLOB FILES *.c *.h)
add_library(si4703 ${FILES})
target_link_libraries(si4703
	 pico_stdlib
	 hardware_i2c
)

target_include_directories(si4703 PUBLIC ./)
