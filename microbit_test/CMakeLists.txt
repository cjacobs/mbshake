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

set (SRC ../source/MicroBitGestureDetector.cpp
         main_stub.cpp
         delayBuffer_test.cpp
         fastmath_test.cpp
         fixed_test.cpp
         fixed_vector_test.cpp
		 iirFilter_test.cpp
		 ringBuffer_test.cpp
		 runningStats_test.cpp
         vector3_test.cpp
         ${PROJ_NAME}.cpp)

set (INCLUDE ../microbit-shake/MicroBitGestureDetector.h
             ../inc/BitUtil.h
             ../inc/DelayBuffer.h
             ../inc/EventThresholdFilter.h
             ../inc/FastMath.h
			 ../inc/FixedPt.h
             ../inc/IirFilter.h
			 ../inc/MicroBitAccess.h
             ../inc/RingBuffer.h
             ../inc/RunningStats.h
             ../inc/Vector3.h
             catch.hpp)
         
source_group("src" FILES ${SRC})
source_group("include" FILES ${INCLUDE})

include_directories(../inc ../microbit-shake)

# create executable
add_executable(${PROJ_NAME} ${SRC} ${INCLUDE})

endif()
