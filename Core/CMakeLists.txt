set(CMAKE_CXX_STANDARD 17)

file(GLOB CORE_SRC_FILES
	src/*.cpp
	)

file(GLOB CORE_HEADER_FILES
	include/*.h
	)

set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")

add_library(GBCore STATIC ${CORE_SRC_FILES} ${CORE_HEADER_FILES})

set_property(TARGET GBCore PROPERTY ARCHIVE_OUTPUT_DIRECTORY ../lib/${CMAKE_VS_PLATFORM_NAME})

set_property(TARGET GBCore PROPERTY COMPILE_DEFINITIONS UNICODE _UNICODE)

target_include_directories(GBCore PRIVATE "$ENV{BOOST_ROOT}" "include" "../curl/include")