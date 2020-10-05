file(GLOB CORE_SRC_FILES
	src/Core.cpp
	)

file(GLOB CORE_HEADER_FILES
	include/Core.h
	)

add_library(core STATIC CORE_SRC_FILES CORE_HEADER_FILES)