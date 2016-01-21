#
# cmake file for print project
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

# define project
set (PROJ_NAME microbit_test)
PROJECT(${PROJ_NAME})

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

