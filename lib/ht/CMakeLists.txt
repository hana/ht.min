project(libhtmin)
cmake_minimum_required(VERSION 4.0.0)

add_library(libhtmin INTERFACE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/oscpkt)

target_include_directories(libhtmin INTERFACE 
	${CMAKE_CURRENT_SOURCE_DIR}/oscpkt
)

target_link_libraries(libhtmin INTERFACE
	liboscpkt
)

target_sources(libhtmin INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/ht_min.h
)

target_compile_features(libhtmin INTERFACE 
	cxx_std_23
	cxx_attribute_deprecated	
)