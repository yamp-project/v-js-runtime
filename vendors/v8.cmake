project("v8")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(RUNTIME_V8_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/vendors/v8)
set(RUNTIME_V8_REPO "v8")
set(RUNTIME_V8_TAG "main")

set(RUNTIME_DEPOT_TOOLS_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/vendors/depot_tools)
set(RUNTIME_DEPOT_TOOLS_REPO "https://chromium.googlesource.com/chromium/tools/depot_tools.git")
set(RUNTIME_DEPOT_TOOLS_TAG "main")

function(depot_tools_update)
    if (WIN32)
        execute_process(
                COMMAND "./update_depot_tools.bat"
                WORKING_DIRECTORY ${RUNTIME_DEPOT_TOOLS_REPO}
        )
    else ()
        execute_process(
                COMMAND "./update_depot_tools"
                WORKING_DIRECTORY ${RUNTIME_DEPOT_TOOLS_REPO}
        )
    endif ()
endfunction()

# Depot Tools
message(STATUS "Setting up depot_tools")

file(GLOB DT_DIR_CONTENT "${RUNTIME_DEPOT_TOOLS_LOCATION}")
if (NOT EXISTS "${RUNTIME_DEPOT_TOOLS_LOCATION}" OR NOT EXISTS ${DT_DIR_CONTENT})
    message(STATUS "Cloning depot_tools")
    file(REMOVE_RECURSE "${RUNTIME_DEPOT_TOOLS_LOCATION}")
    file(MAKE_DIRECTORY "${RUNTIME_DEPOT_TOOLS_LOCATION}")
    execute_process(
            COMMAND "git" "clone" "${RUNTIME_DEPOT_TOOLS_REPO}" "${RUNTIME_DEPOT_TOOLS_LOCATION}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_CLONE_RESULT
    )
    if(NOT GIT_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone repository: ${RUNTIME_DEPOT_TOOLS_REPO}")
    else()
        message(STATUS "Successfully cloned repository: ${RUNTIME_DEPOT_TOOLS_REPO}")
    endif()

    execute_process(
            COMMAND "git" "checkout" "-B" "origin/${RUNTIME_DEPOT_TOOLS_TAG}"
            WORKING_DIRECTORY ${RUNTIME_DEPOT_TOOLS_LOCATION}
            RESULT_VARIABLE GIT_CHECKOUT_RESULT
    )

    if(NOT GIT_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to switch to tag: ${RUNTIME_DEPOT_TOOLS_TAG}")
    else()
        message(STATUS "Successfully switched tag: ${RUNTIME_DEPOT_TOOLS_TAG}")
        depot_tools_update()
    endif()
else ()
    message(STATUS "Updating depot_tools")
    execute_process(
            COMMAND "git" "fetch"
            COMMAND "git" "checkout" "-B" "origin/${RUNTIME_DEPOT_TOOLS_TAG}"
            COMMAND "git" "pull" "${RUNTIME_DEPOT_TOOLS_REPO}"
            WORKING_DIRECTORY ${RUNTIME_DEPOT_TOOLS_LOCATION}
            RESULT_VARIABLE GIT_UPDATE_RESULT
    )
    if(NOT GIT_UPDATE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to update repository: ${RUNTIME_DEPOT_TOOLS_REPO}")
    else()
        message(STATUS "Successfully updated repository: ${RUNTIME_DEPOT_TOOLS_REPO}")
        depot_tools_update()
    endif()
endif ()

# V8
file(GLOB V8_DIR_CONTENT "${RUNTIME_V8_LOCATION}")
if (NOT EXISTS "${RUNTIME_V8_LOCATION}" OR NOT EXISTS ${V8_DIR_CONTENT})
    message(STATUS "Setting up v8")
    file(REMOVE_RECURSE ${RUNTIME_V8_LOCATION})
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/vendors/.cipd)
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/vendors/.gclient)
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/vendors/.gcs_entries)
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/vendors/.gclient_entries)
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/vendors/.gclient_previous_sync_commits)

    message(STATUS "Start v8 download")
    execute_process(
            COMMAND "${RUNTIME_DEPOT_TOOLS_LOCATION}/fetch" "--no-history" "${RUNTIME_V8_REPO}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/vendors
            RESULT_VARIABLE DT_FETCH_RESULT
    )
    if(NOT DT_FETCH_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to fetch: ${RUNTIME_V8_REPO}")
    else()
        message(STATUS "Successfully fetched: ${RUNTIME_V8_REPO}")
    endif()
else ()
    message(STATUS "Updating v8")

    execute_process(
            COMMAND "git" "checkout" "-B" "origin/${RUNTIME_V8_TAG}"
            COMMAND "git" "pull" "origin/${RUNTIME_V8_TAG}"
            COMMAND "${RUNTIME_DEPOT_TOOLS_LOCATION}/gclient" "sync"
            WORKING_DIRECTORY ${RUNTIME_V8_LOCATION}
    )
endif ()

# V8 build process
option(BUILD_V8 "Build V8 Javascript engine" OFF)

if (${BUILD_V8})
    message(STATUS "Building V8")

    if (WIN32)
        set(GN_EXECUTABLE "${RUNTIME_DEPOT_TOOLS_LOCATION}/gn.bat")
    else ()
        set(GN_EXECUTABLE "${RUNTIME_DEPOT_TOOLS_LOCATION}/gn")
    endif ()

    set(TREAT_WARNINGS_AS_ERRORS "false")
    set(IS_OFFICIAL_BUILD "false")
    set(IS_COMPONENT_BUILD "false")
    set(IS_DEBUG "false")
    set(SYMBOL_LEVEL "0")
    set(STRIP_DEBUG_INFO "true")
    set(USE_CUSTOM_LIBCXX "false")
    set(V8_MONOLITHIC "true")
    set(V8_ENABLE_SANDBOX "true")
    set(V8_ENABLE_POINTER_COMPRESSION "true")
    set(V8_ENABLE_WEBASSEMBLY "true")
    set(V8_ENABLE_GDBJIT "false")
    set(V8_ENABLE_I18N_SUPPORT "true")
    set(V8_ENABLE_TEST_FEATURES "false")
    set(V8_USE_EXTERNAL_STARTUP_DATA "false")

    if(WIN32)
        set(IS_CLANG "false")
    else ()
        set(USE_SYSROOT "false")
        set(IS_CLANG "true")
    endif()

    set(V8_BUILD_DIR "out.gn")

    set(GN_ARGS
            "treat_warnings_as_errors=${TREAT_WARNINGS_AS_ERRORS}"
            "is_official_build=${IS_OFFICIAL_BUILD}"
            "is_component_build=${IS_COMPONENT_BUILD}"
            "is_debug=${IS_DEBUG}"
            "symbol_level=${SYMBOL_LEVEL}"
            "strip_debug_info=${STRIP_DEBUG_INFO}"
            "use_custom_libcxx=${USE_CUSTOM_LIBCXX}"
            "v8_monolithic=${V8_MONOLITHIC}"
            "v8_enable_sandbox=${V8_ENABLE_SANDBOX}"
            "v8_enable_pointer_compression=${V8_ENABLE_POINTER_COMPRESSION}"
            "v8_enable_webassembly=${V8_ENABLE_WEBASSEMBLY}"
            "v8_enable_gdbjit=${V8_ENABLE_GDBJIT}"
            "v8_enable_i18n_support=${V8_ENABLE_I18N_SUPPORT}"
            "v8_enable_test_features=${V8_ENABLE_TEST_FEATURES}"
            "v8_use_external_startup_data=${V8_USE_EXTERNAL_STARTUP_DATA}"
    )

    if(WIN32)
        list(APPEND GN_ARGS "is_clang=${IS_CLANG}")
    else ()
        list(APPEND GN_ARGS "use_sysroot=${USE_SYSROOT}")
        list(APPEND GN_ARGS "is_clang=${IS_CLANG}")
    endif()

    string(REPLACE ";" " " GN_ARGS_STRING "${GN_ARGS}")

    find_program(NINJA_EXECUTABLE ninja REQUIRED)

    add_custom_target(gn_generate
        COMMAND ${CMAKE_COMMAND} -E echo "Running gn build..."
        COMMAND ${GN_EXECUTABLE} gen ${V8_BUILD_DIR} --args=${GN_ARGS_STRING}
        WORKING_DIRECTORY ${RUNTIME_V8_LOCATION}
        COMMENT "Generating V8 build files with GN"
    )

    add_custom_target(v8_build
            COMMAND ${NINJA_EXECUTABLE} -C ${V8_BUILD_DIR} v8_monolith
            WORKING_DIRECTORY ${RUNTIME_V8_LOCATION}
            DEPENDS gn_generate
            COMMENT "Building V8 with Ninja"
    )

    add_custom_target(build_v8
            COMMAND ${CMAKE_COMMAND} -E echo "Finished building V8 from source."
            DEPENDS v8_build
            COMMENT "Complete V8 build process"
    )

    add_library(V8::V8 SHARED IMPORTED GLOBAL)
    if(WIN32)
        set(V8_LIBRARY_NAME "v8_monolith.lib")
        set(V8_LIBRARY_PATH "${RUNTIME_V8_LOCATION}/${V8_BUILD_DIR}/obj/v8_monolith.lib")
    else()
        set(V8_LIBRARY_NAME "libv8_monolith.a")
        set(V8_LIBRARY_PATH "${RUNTIME_V8_LOCATION}/${V8_BUILD_DIR}/obj/libv8_monolith.a")
    endif()

    set_target_properties(V8::V8 PROPERTIES
            IMPORTED_LOCATION "${V8_LIBRARY_PATH}"
            INTERFACE_INCLUDE_DIRECTORIES "${RUNTIME_V8_LOCATION}/include"
    )

    add_dependencies(V8::V8 build_v8)
else ()
    message(STATUS "V8 build disabled. Use -DBUILD_V8=ON to enable.")
endif ()