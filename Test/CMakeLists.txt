cmake_minimum_required(VERSION 3.20)

project("v-js-runtime" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

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

file(GLOB_RECURSE PROJECT_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../Client/source/*.cpp)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

# if(MSVC)
#     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MD /NODEFAULTLIB:MSVCRT.LIB")
# endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Client/source)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Vendors/temp)

target_link_libraries(${PROJECT_NAME} PUBLIC YampFramework yamp::sdk v8helper::v8helper)
target_link_libraries(${PROJECT_NAME} PUBLIC shlwapi.lib winmm.lib DbgHelp.lib advapi32.lib)

target_link_directories(${PROJECT_NAME} PUBLIC ../Vendors/temp)
target_link_libraries(${PROJECT_NAME} PUBLIC v8_monolith.lib)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_compile_definitions(${PROJECT_NAME} PUBLIC
    V8_COMPRESS_POINTERS=1
    V8_31BIT_SMIS_ON_64BIT_ARCH=1
    V8_IMMINENT_DEPRECATION_WARNINGS=1
    V8_ENABLE_SANDBOX=1
)