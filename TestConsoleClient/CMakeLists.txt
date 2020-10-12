set(CMAKE_CXX_STANDARD 17)

include_directories("include")

file(GLOB TEST_CLIENT_SRC_FILES
	src/main.cpp
	)

file(GLOB TEST_CLIENT_HEADER_FILES
	include/stdafx.h
	)

add_executable(TestConsoleClient ${TEST_CLIENT_SRC_FILES} ${TEST_CLIENT_HEADER_FILES})

set_property(TARGET TestConsoleClient PROPERTY COMPILE_DEFINITIONS _LIB UNICODE _UNICODE $<IF:$<CONFIG:Debug>,_DEBUG,NDEBUG>)

target_link_libraries(TestConsoleClient ws2_32)
target_link_libraries(TestConsoleClient Mswsock)