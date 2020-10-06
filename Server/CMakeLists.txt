include_directories("include")

file(GLOB SERVER_SRC_FILES
	src/main.cpp
	)

file(GLOB SERVER_HEADER_FILES
	include/Constant.h
	)

add_executable(CatchMyTailServer ${SERVER_SRC_FILES} ${SERVER_HEADER_FILES})

target_link_libraries(CatchMyTailServer ../lib/${CMAKE_VS_PLATFORM_NAME}/${CMAKE_CFG_INTDIR}/ServerCore)