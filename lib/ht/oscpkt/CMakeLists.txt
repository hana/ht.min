project(liboscpkt)
cmake_minimum_required(VERSION 4.0.0)

add_library(liboscpkt INTERFACE)

target_sources(liboscpkt INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/oscpkt.hh
)

target_compile_features(liboscpkt INTERFACE 
	cxx_std_23
	cxx_attribute_deprecated	
)