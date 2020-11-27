set(CMAKE_CXX_STANDARD 17)

include_directories("include")

file(GLOB TEST_CLIENT_SRC_FILES
	src/*.cpp
	)

file(GLOB TEST_CLIENT_HEADER_FILES
	include/*.h
	)

add_executable(GBClient ${TEST_CLIENT_SRC_FILES} ${TEST_CLIENT_HEADER_FILES})

set_property(TARGET GBClient PROPERTY COMPILE_DEFINITIONS _LIB UNICODE _UNICODE $<IF:$<CONFIG:Debug>,_DEBUG,NDEBUG>)

target_link_libraries(GBClient ws2_32)
target_link_libraries(GBClient Mswsock)