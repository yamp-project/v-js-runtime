cmake_minimum_required(VERSION 3.20)

project("v-js-runtime-build")

set(YAMP_FRAMEWORK_DISABLE_DISCORDRPC ON)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Target architecture: x64")
else()
    message(FATAL_ERROR "Unsupported architecture")
endif()

add_subdirectory("Client")
# add_subdirectory("Test")
add_subdirectory("Vendors")
