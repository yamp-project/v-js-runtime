project("v-js-runtime-shared" CXX)

file(GLOB_RECURSE PROJECT_SOURCE_FILES "src/*.h" "src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC yamp::sdk)