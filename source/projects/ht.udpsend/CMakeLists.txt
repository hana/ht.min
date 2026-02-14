# Copyright 2018 The Min-DevKit Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.20)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)


#############################################################
# MAX EXTERNAL
#############################################################

# message(STATUS "C74_INCLUDES: ${C74_INCLUDES}")
include_directories( 
	"${C74_INCLUDES}"
	${CMAKE_CURRENT_SOURCE_DIR}
)


set(SOURCE_FILES
	${PROJECT_NAME}.cpp
	udp_sender.cpp
)

add_library( 
	${PROJECT_NAME} 
	MODULE
	${SOURCE_FILES}
)

target_compile_features(${PROJECT_NAME} PUBLIC 
	cxx_std_23
	cxx_attribute_deprecated	
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/oscpp/include
    ${CMAKE_SOURCE_DIR}/lib/oscpkt
    ${CMAKE_SOURCE_DIR}/lib/ht
)

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)

set(CMAKE_C_STANDARD 20)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#############################################################
# UNIT TEST
#############################################################

include(${C74_MIN_API_DIR}/test/min-object-unittest.cmake)
