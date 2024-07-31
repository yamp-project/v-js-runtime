cmake_minimum_required(VERSION 3.20)

project("v-js-runtime" LANGUAGES CXX)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Target architecture: x64")
else()
    message(FATAL_ERROR "Unsupported architecture")
endif()

if(UNIX)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/linux64/${CMAKE_BUILD_TYPE})
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/win64/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/win64/${CMAKE_BUILD_TYPE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/win64/${CMAKE_BUILD_TYPE})
endif()

file(GLOB_RECURSE PROJECT_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)
target_link_libraries(${PROJECT_NAME} PUBLIC yamp::sdk)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
