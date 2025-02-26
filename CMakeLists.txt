cmake_minimum_required(VERSION 3.22)

project(CacheAnalysis)

# We can only compile on GCC on Linux
if(NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
  message(FATAL_ERROR "This project only supports Linux with GCC.")
endif()

set(CMAKE_C_STANDARD 23)

include_directories(include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source Files
set(SRCS src/eviction_set.c src/occupancy_profile.c)

# Optimization Flags
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) # LTO
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1")
# set(CMAKE_BUILD_TYPE Debug) 

# Analysis Application
add_executable(main main.c ${SRCS}) 
add_executable(timing timing.c ${SRCS}) 
add_executable(papp papp.c ${SRCS}) 
