project("v-js-runtime-shared")

file(GLOB_RECURSE PROJECT_SOURCE_FILES "src/*.h")

add_library(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} INTERFACE "src")

target_link_libraries(${PROJECT_NAME} PRIVATE INTERFACE yamp::sdk)