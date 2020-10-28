set(CMAKE_CXX_STANDARD 17)

file(GLOB CURL_TEST_SRC
	*.cpp
	)

file(GLOB CURL_TEST_HEADER
	*.h
	)

add_executable(CurlTest ${CURL_TEST_SRC} ${CURL_TEST_HEADER})

set_property(TARGET CurlTest PROPERTY COMPILE_DEFINITIONS UNICODE _UNICODE)

target_include_directories(CurlTest PRIVATE "../../curl/include")