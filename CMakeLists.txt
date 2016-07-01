cmake_minimum_required(VERSION 2.8.11)
cmake_policy(SET CMP0015 NEW)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/sdsl-lite/CMakeModules")
include(AppendCompilerFlags)
include(ExternalProject)

project(AUTOCOMPLETE CXX C)

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/include
                    ${CMAKE_HOME_DIRECTORY}/external/cppzmq/
                    ${CMAKE_HOME_DIRECTORY}/external/zeromq/include/
                    ${CMAKE_BINARY_DIR}/external/sdsl-lite/include
                    ${CMAKE_BINARY_DIR}/external/sdsl-lite/external/libdivsufsort-2.0.1/include
                   )

LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/external/sdsl-lite/lib
                 ${CMAKE_BINARY_DIR}/external/zeromq/lib
                )

APPEND_CXX_COMPILER_FLAGS("-msse4.2 -std=c++11 -Wall -DNDEBUG" "GCC" CMAKE_CXX_FLAGS)
APPEND_CXX_COMPILER_FLAGS("-O3 -ffast-math -funroll-loops" "GCC" CMAKE_CXX_FLAGS)
APPEND_CXX_COMPILER_FLAGS("-msse4.2 -std=c++11 -g -funroll-loops -DNDEBUG -stdlib=libc++" "CLANG" CMAKE_CXX_FLAGS)


ADD_SUBDIRECTORY(external/zeromq)
SET_PROPERTY(DIRECTORY external/zeromq PROPERTY ZMQ_BUILD_TESTS FALSE)

ADD_SUBDIRECTORY(external/sdsl-lite)


ADD_EXECUTABLE(index src/index.cpp)
TARGET_LINK_LIBRARIES(index sdsl divsufsort divsufsort64 pthread)
