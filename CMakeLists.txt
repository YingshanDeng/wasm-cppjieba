PROJECT(CPPJIEBA)

CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/cppjieba/deps
    ${PROJECT_SOURCE_DIR}/cppjieba/include
)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr/local/cppjieba" CACHE PATH "default install path" FORCE )
endif()

ADD_DEFINITIONS(-O3 -Wall -g)
IF(APPLE) # mac os
    ADD_DEFINITIONS(-std=c++0x)
endif()

ADD_SUBDIRECTORY(cppjieba/deps)
ADD_EXECUTABLE(wasm-cppjieba src/wasm-cppjieba.cpp)
