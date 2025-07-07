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
    if (UNIX)
        execute_process(
                COMMAND "./update_depot_tools"
                WORKING_DIRECTORY ${RUNTIME_DEPOT_TOOLS_REPO}
        )
    else ()
        execute_process(
                COMMAND "./update_depot_tools.bat"
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
            COMMAND "git" "checkout" "origin/${RUNTIME_V8_TAG}"
            COMMAND "git" "pull"
            COMMAND "${RUNTIME_DEPOT_TOOLS_LOCATION}/gclient" "sync"
            WORKING_DIRECTORY ${RUNTIME_V8_LOCATION}
    )
endif ()

