set(CMAKE_CXX_STANDARD 17)

include_directories("include")

add_link_options(/STACK:1073741824)

file(GLOB SERVER_SRC_FILES
	src/main.cpp
	)

file(GLOB SERVER_HEADER_FILES
	include/Constant.h
	)

add_executable(CatchMyTailServer ${SERVER_SRC_FILES} ${SERVER_HEADER_FILES})

target_link_libraries(CatchMyTailServer ../lib/${CMAKE_VS_PLATFORM_NAME}/${CMAKE_CFG_INTDIR}/ServerCore)

set_property(TARGET CatchMyTailServer PROPERTY COMPILE_DEFINITIONS _LIB UNICODE _UNICODE $<IF:$<CONFIG:Debug>,_DEBUG,NDEBUG>)

target_link_libraries(CatchMyTailServer ws2_32)
target_link_libraries(CatchMyTailServer Mswsock)