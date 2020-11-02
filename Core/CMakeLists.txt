set(CMAKE_CXX_STANDARD 17)

file(GLOB CORE_SRC_FILES
	src/ServerCore.cpp
	src/stdafx.cpp
	src/GBBuffer.cpp
	src/Constant.cpp
	)

file(GLOB CORE_HEADER_FILES
	include/ServerCore.h
	include/stdafx.h
	include/GBString.h
	include/GBBuffer.h
	include/Constant.h
	)

set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")

add_library(ServerCore STATIC ${CORE_SRC_FILES} ${CORE_HEADER_FILES})

set_property(TARGET ServerCore PROPERTY ARCHIVE_OUTPUT_DIRECTORY ../lib/${CMAKE_VS_PLATFORM_NAME})

set_property(TARGET ServerCore PROPERTY COMPILE_DEFINITIONS UNICODE _UNICODE)

target_include_directories(ServerCore PRIVATE "$ENV{BOOST_ROOT}" "include" "../curl/include")