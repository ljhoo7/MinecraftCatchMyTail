set(CMAKE_CXX_STANDARD 17)

include_directories("include")

#add_link_options(/STACK:10737418)

file(GLOB SERVER_SRC_FILES
	src/main.cpp
	)

file(GLOB SERVER_HEADER_FILES
	include/Constant.h
	)

add_executable(GBServer ${SERVER_SRC_FILES} ${SERVER_HEADER_FILES})

target_link_libraries(GBServer ../lib/${CMAKE_VS_PLATFORM_NAME}/${CMAKE_CFG_INTDIR}/ServerCore)

set_property(TARGET GBServer PROPERTY COMPILE_DEFINITIONS _LIB UNICODE _UNICODE $<IF:$<CONFIG:Debug>,_DEBUG,NDEBUG>)

target_link_libraries(GBServer ws2_32)
target_link_libraries(GBServer Mswsock)