cmake_minimum_required (VERSION 2.6)

project (BisonFlexTest)

set(CMAKE_CXX_STANDARD 17)

file(GLOB BISON_FLEX_TEST_SRC
	*.cpp
	)

file(GLOB BISON_FLEX_TEST_HEADER
	*.h
	)

add_executable(BisonFlexTest ${BISON_FLEX_TEST_SRC} ${BISON_FLEX_TEST_HEADER})

set_property(TARGET BisonFlexTest PROPERTY COMPILE_DEFINITIONS UNICODE _UNICODE)

#target_include_directories(BisonFlexTest PRIVATE "")