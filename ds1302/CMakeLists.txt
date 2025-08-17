file(GLOB FILES *.c *.h)
add_library(ds1302 ${FILES})
target_link_libraries(ds1302
	 pico_stdlib
)

target_include_directories(ds1302 PUBLIC ./)
