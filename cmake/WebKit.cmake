include(ExternalProject)

function(setup_webkit_jsc)
    find_program(PERL_EXECUTABLE perl REQUIRED)
    find_program(RUBY_EXECUTABLE ruby REQUIRED)
    find_program(NINJA_EXECUTABLE ninja REQUIRED)

    if(WIN32)
        find_program(PYTHON_EXECUTABLE python python3 REQUIRED)
    else()
        find_program(PYTHON_EXECUTABLE python3 python REQUIRED)
    endif()

    message(STATUS "Found Perl: ${PERL_EXECUTABLE}")
    message(STATUS "Found Python: ${PYTHON_EXECUTABLE}")
    message(STATUS "Found Ruby: ${RUBY_EXECUTABLE}")
    message(STATUS "Found Ninja: ${NINJA_EXECUTABLE}")

    if(UNIX AND NOT APPLE)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(ICU REQUIRED icu-uc icu-i18n)
        pkg_check_modules(GLIB2 REQUIRED glib-2.0)
        message(STATUS "Found ICU: ${ICU_VERSION}")
        message(STATUS "Found GLib: ${GLIB2_VERSION}")
    endif()

    if(WIN32)
        set(WEBKIT_PORT "WinCairo")
        set(JSC_LIBRARY_NAME "JavaScriptCore.lib")
        set(JSC_LIBRARY_SUBDIR "lib")
        set(WEBKIT_CMAKE_ARGS -DUSE_WINCAIRO=ON -DCMAKE_SYSTEM_VERSION=10.0)
    elseif(APPLE)
        set(WEBKIT_PORT "Mac")
        set(JSC_LIBRARY_NAME "libJavaScriptCore.a")
        set(JSC_LIBRARY_SUBDIR "lib")
        set(WEBKIT_CMAKE_ARGS -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15)
    else()
        set(WEBKIT_PORT "GTK")
        set(JSC_LIBRARY_NAME "libJavaScriptCore.a")
        set(JSC_LIBRARY_SUBDIR "lib")
        set(WEBKIT_CMAKE_ARGS -DUSE_GTK4=OFF -DUSE_LIBSECRET=OFF)
    endif()

    set(WEBKIT_INSTALL_DIR "${CMAKE_BINARY_DIR}/webkit-install")
    set(WEBKIT_BUILD_DIR "${CMAKE_BINARY_DIR}/webkit-build")
    set(WEBKIT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/vendors/webkit")
    set(JSC_LIBRARY_PATH "${WEBKIT_INSTALL_DIR}/${JSC_LIBRARY_SUBDIR}/${JSC_LIBRARY_NAME}")

    if(NOT EXISTS "${WEBKIT_SOURCE_DIR}/Source/JavaScriptCore/CMakeLists.txt")
        message(FATAL_ERROR "WebKit source not found. Please run: git submodule update --init --recursive")
    endif()

    set(JSC_INCLUDE_DIR "${WEBKIT_SOURCE_DIR}/Source/JavaScriptCore/API")

    ExternalProject_Add(WebKit_External
            SOURCE_DIR ${WEBKIT_SOURCE_DIR}
            BINARY_DIR ${WEBKIT_BUILD_DIR}
            INSTALL_DIR ${WEBKIT_INSTALL_DIR}
            CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DCMAKE_GENERATOR=Ninja
            -DPORT=${WEBKIT_PORT}
            -DENABLE_STATIC_JSC=ON
            -DENABLE_WEBKIT=OFF
            -DENABLE_WEBCORE=OFF
            -DENABLE_TOOLS=OFF
            -DENABLE_MINIBROWSER=OFF
            -DENABLE_WEBDRIVER=OFF
            -DENABLE_API_TESTS=OFF
            -DENABLE_VIDEO=OFF
            -DENABLE_WEB_AUDIO=OFF
            ${WEBKIT_CMAKE_ARGS}
            BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target JavaScriptCore --config ${CMAKE_BUILD_TYPE} --parallel
            INSTALL_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target install --config ${CMAKE_BUILD_TYPE}
            UPDATE_COMMAND ""
            LOG_CONFIGURE ON
            LOG_BUILD ON
            LOG_INSTALL ON
    )

    add_library(JavaScriptCore STATIC IMPORTED GLOBAL)
    file(MAKE_DIRECTORY "${JSC_INCLUDE_DIR}")
    file(MAKE_DIRECTORY "${WEBKIT_INSTALL_DIR}/${JSC_LIBRARY_SUBDIR}")

    string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
    set_property(TARGET JavaScriptCore APPEND PROPERTY IMPORTED_CONFIGURATIONS ${CMAKE_BUILD_TYPE_UPPER})

    set_target_properties(JavaScriptCore PROPERTIES
            IMPORTED_LOCATION_${CMAKE_BUILD_TYPE_UPPER} "${JSC_LIBRARY_PATH}"
            INTERFACE_INCLUDE_DIRECTORIES "${JSC_INCLUDE_DIR}"
    )
    add_dependencies(JavaScriptCore WebKit_External)

    add_library(JavaScriptCore_Dependencies INTERFACE)
    if(WIN32)
        target_link_libraries(JavaScriptCore_Dependencies INTERFACE
                winmm user32 ole32 oleaut32 psapi version shlwapi dbghelp ws2_32 iphlpapi
        )
    elseif(APPLE)
        target_link_libraries(JavaScriptCore_Dependencies INTERFACE
                "-framework Foundation" "-framework CoreFoundation" "-framework Security"
        )
    else()
        target_link_libraries(JavaScriptCore_Dependencies INTERFACE
                ${ICU_LIBRARIES} ${GLIB2_LIBRARIES} pthread dl m atomic
        )
        target_include_directories(JavaScriptCore_Dependencies INTERFACE
                ${ICU_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIRS}
        )
    endif()

    add_library(JavaScriptCore_Complete INTERFACE)
    target_link_libraries(JavaScriptCore_Complete INTERFACE
            JavaScriptCore
            JavaScriptCore_Dependencies
    )
    add_library(JavaScriptCore::Core ALIAS JavaScriptCore_Complete)

    message(STATUS "JSC Include directory: ${JSC_INCLUDE_DIR}")
endfunction()