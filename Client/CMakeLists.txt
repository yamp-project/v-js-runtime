cmake_minimum_required(VERSION 3.20)

project("v-js-runtime" LANGUAGES CXX)

if(UNIX)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/linux64/${CMAKE_BUILD_TYPE})
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/win64/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/win64/${CMAKE_BUILD_TYPE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/win64/${CMAKE_BUILD_TYPE})
endif()

# Gather source files
file(GLOB_RECURSE PROJECT_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

# Create shared library target
add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCE_FILES})

# Apply precompiled headers directly to the project target
target_precompile_headers(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source/stdafx.h)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)

# Link external libraries
target_link_libraries(${PROJECT_NAME} PUBLIC framework::framework yamp::sdk v8helper::v8helper)
target_link_libraries(${PROJECT_NAME} PUBLIC shlwapi.lib winmm.lib DbgHelp.lib advapi32.lib)

# Set C++ standard and compile definitions
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_compile_definitions(${PROJECT_NAME} PUBLIC
    NOMINMAX
    WIN32_LEAN_AND_MEAN    
    VC_EXTRALEAN
    V8_COMPRESS_POINTERS=1
    V8_31BIT_SMIS_ON_64BIT_ARCH=1
    V8_IMMINENT_DEPRECATION_WARNINGS=1
    V8_ENABLE_SANDBOX=1
)
