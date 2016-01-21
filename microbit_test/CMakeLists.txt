#
# cmake file for microbit_test project
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

set (PROJ_NAME microbit_test)

# define project
PROJECT(${PROJ_NAME})

if("${YOTTA_BUILD_YEAR}")

add_library(${PROJ_NAME} empty_main.cpp)
set_target_properties(${PROJ_NAME} PROPERTIES LINKER_LANGUAGE CXX)

else()

#enable C++11 in GCC, etc
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
  add_compile_options(-std=c++1y)
endif()

set (SRC ../source/classifiers.cpp
         main_stub.cpp
         ${PROJ_NAME}.cpp)

set (INCLUDE ../source/classifiers.h
             ../source/delayBuffer.h
             ../source/eventThresholdFilter.h
             ../source/fastmath.h
             ../source/iirFilter.h
             ../source/ringBuffer.h
             ../source/runningStats.h
             ../source/vec3.h
             catch.hpp)
         
source_group("src" FILES ${SRC})
source_group("include" FILES ${INCLUDE})

include_directories(../source)

# create executable
add_executable(${PROJ_NAME} ${SRC} ${INCLUDE})

endif()