file(GLOB FILES *.c *.h)
add_library(momefilo_flash ${FILES})
target_link_libraries(momefilo_flash
	hardware_flash
	hardware_sync
	pico_stdlib
)

target_include_directories(momefilo_flash PUBLIC ./)
