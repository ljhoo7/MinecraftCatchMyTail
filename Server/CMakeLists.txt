file(GLOB SERVER_SRC_FILES
	src/main.cpp
	)

file(GLOB SERVER_HEADER_FILES
	include/Constant.h
	)

add_library(server STATIC ${SERVER_SRC_FILES} ${SERVER_HEADER_FILES})