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

    if(RUBY_EXECUTABLE)
        execute_process(
                COMMAND ${RUBY_EXECUTABLE} -e "puts RUBY_VERSION"
                OUTPUT_VARIABLE RUBY_VERSION_OUTPUT
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
                RESULT_VARIABLE RUBY_VERSION_RESULT
        )

        if(RUBY_VERSION_RESULT EQUAL 0 AND RUBY_VERSION_OUTPUT)
            string(REPLACE "." ";" RUBY_VERSION_LIST ${RUBY_VERSION_OUTPUT})
            list(GET RUBY_VERSION_LIST 0 RUBY_MAJOR)
            list(GET RUBY_VERSION_LIST 1 RUBY_MINOR)

            if(RUBY_MAJOR LESS 2 OR (RUBY_MAJOR EQUAL 2 AND RUBY_MINOR LESS 5))
                message(FATAL_ERROR "Ruby 2.5 or higher is required. Found Ruby ${RUBY_VERSION_OUTPUT}")
            endif()

            message(STATUS "Found Ruby: ${RUBY_EXECUTABLE} (version ${RUBY_VERSION_OUTPUT})")
        else()
            message(FATAL_ERROR "Could not determine Ruby version. Please ensure Ruby 2.5+ is installed.")
        endif()
    endif()

    message(STATUS "Found Perl: ${PERL_EXECUTABLE}")
    message(STATUS "Found Python: ${PYTHON_EXECUTABLE}")
    message(STATUS "Found Ninja: ${NINJA_EXECUTABLE}")

    # Set up directories
    set(WEBKIT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/vendors/webkit")
    set(WEBKIT_BUILD_DIR "${CMAKE_BINARY_DIR}/webkit-build")
    set(WEBKIT_INSTALL_DIR "${CMAKE_BINARY_DIR}/webkit-install")

    if(WIN32)
        set(WEBKIT_PORT "Win")
        set(JSC_LIBRARY_NAME "JavaScriptCore.lib")
        set(JSC_LIBRARY_SUBDIR "lib")

        set(WEBKIT_CLANG_ARGS)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            execute_process(
                    COMMAND ${CMAKE_CXX_COMPILER} --print-resource-dir
                    OUTPUT_VARIABLE CLANG_RESOURCE_DIR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET
            )

            if(CLANG_RESOURCE_DIR)
                find_library(CLANG_BUILTINS_LIBRARY
                        NAMES clang_rt.builtins-x86_64 clang_rt.builtins_x86_64 clang_rt.builtins
                        PATHS
                        "${CLANG_RESOURCE_DIR}/lib/windows"
                        "${CLANG_RESOURCE_DIR}/lib"
                        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/lib/clang/19/lib/windows"
                        "C:/Program Files/Microsoft Visual Studio/2022/Professional/VC/Tools/Llvm/x64/lib/clang/19/lib/windows"
                        "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/Llvm/x64/lib/clang/19/lib/windows"
                        NO_DEFAULT_PATH
                )

                if(CLANG_BUILTINS_LIBRARY)
                    message(STATUS "Found Clang builtins: ${CLANG_BUILTINS_LIBRARY}")
                    list(APPEND WEBKIT_CLANG_ARGS -DCLANG_BUILTINS_LIBRARY=${CLANG_BUILTINS_LIBRARY})
                else()
                    list(APPEND WEBKIT_CLANG_ARGS -DUSE_CLANG_BUILTINS=OFF)
                endif()
            else()
                list(APPEND WEBKIT_CLANG_ARGS -DUSE_CLANG_BUILTINS=OFF)
            endif()
        endif()
    elseif(APPLE)
        set(WEBKIT_PORT "Mac")
        set(JSC_LIBRARY_NAME "libJavaScriptCore.a")
        set(JSC_LIBRARY_SUBDIR "lib")
        set(WEBKIT_CLANG_ARGS)
    else()
        set(WEBKIT_PORT "GTK")
        set(JSC_LIBRARY_NAME "libJavaScriptCore.a")
        set(JSC_LIBRARY_SUBDIR "lib")
        set(WEBKIT_CLANG_ARGS)
    endif()

    set(JSC_LIBRARY_PATH "${WEBKIT_INSTALL_DIR}/${JSC_LIBRARY_SUBDIR}/${JSC_LIBRARY_NAME}")

    if(NOT EXISTS "${WEBKIT_SOURCE_DIR}/Source/JavaScriptCore/CMakeLists.txt")
        message(FATAL_ERROR "Patched WebKit source not found at ${WEBKIT_SOURCE_DIR}. Please ensure your WebKit fork is properly set up as a submodule.")
    endif()

    set(JSC_CLEAN_INCLUDE_DIR "${CMAKE_BINARY_DIR}/jsc-include")
    file(MAKE_DIRECTORY "${JSC_CLEAN_INCLUDE_DIR}/JavaScriptCore")

    if(WIN32 OR UNIX AND NOT APPLE)
        file(COPY "${WEBKIT_SOURCE_DIR}/Source/JavaScriptCore/API/"
                DESTINATION "${JSC_CLEAN_INCLUDE_DIR}/JavaScriptCore/"
                FILES_MATCHING PATTERN "*.h"
                PATTERN "*CF.h" EXCLUDE)

        file(READ "${JSC_CLEAN_INCLUDE_DIR}/JavaScriptCore/JavaScriptCore.h" JAVACORE_CONTENT)
        string(REPLACE "#include <JavaScriptCore/JSStringRefCF.h>" "" JAVACORE_CONTENT "${JAVACORE_CONTENT}")
        file(WRITE "${JSC_CLEAN_INCLUDE_DIR}/JavaScriptCore/JavaScriptCore.h" "${JAVACORE_CONTENT}")
    else()
        file(COPY "${WEBKIT_SOURCE_DIR}/Source/JavaScriptCore/API/"
                DESTINATION "${JSC_CLEAN_INCLUDE_DIR}/JavaScriptCore/"
                FILES_MATCHING PATTERN "*.h")
    endif()

    set(JSC_INCLUDE_DIR "${JSC_CLEAN_INCLUDE_DIR}")

    include(${CMAKE_SOURCE_DIR}/cmake/ICU.cmake)

    ExternalProject_Add(WebKit_External
            SOURCE_DIR ${WEBKIT_SOURCE_DIR}
            BINARY_DIR ${WEBKIT_BUILD_DIR}
            INSTALL_DIR ${WEBKIT_INSTALL_DIR}
            DEPENDS ICU_External
            CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DCMAKE_GENERATOR=Ninja
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_MAKE_PROGRAM=${NINJA_EXECUTABLE}
            -DPORT=${WEBKIT_PORT}
            -DRUBY_EXECUTABLE=${RUBY_EXECUTABLE}
            -DPERL_EXECUTABLE=${PERL_EXECUTABLE}
            -DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}
            -DICU_ROOT=${ICU_ROOT}

            # Core JSC-only configuration
            -DENABLE_STATIC_JSC=ON
            -DENABLE_XSLT=OFF
            -DUSE_SKIA=OFF
            -DUSE_CAIRO=OFF
            -DENABLE_INTL=ON
            -DENABLE_WEBGL=OFF
            -DUSE_CAPSTONE=OFF
            -DUSE_ANGLE_EGL=OFF
            -DUSE_SYSTEM_UNIFDEF=OFF
            -DENABLE_WEBKIT=OFF
            -DENABLE_WEBCORE=OFF
            -DENABLE_WEBKIT_LEGACY=OFF
            -DENABLE_LAYOUT_TESTS=OFF
            -DENABLE_TOOLS=OFF
            -DENABLE_MINIBROWSER=OFF
            -DENABLE_WEBDRIVER=OFF
            -DENABLE_WEBGPU=OFF
            -DENABLE_API_TESTS=OFF
            -DUSE_XDGMIME=OFF
            -DUSE_LIBWEBRTC=OFF
            -DUSE_SYSTEM_SYSPROF_CAPTURE=OFF
            -DUSE_SYSPROF_CAPTURE=OFF
            -DENABLE_WEBINSPECTORUI=OFF
            -DUSE_CF=OFF

            # Disable graphics and imaging
            -DENABLE_GRAPHICS=OFF
            -DUSE_ANGLE=OFF
            -DUSE_ZLIB=OFF
            -DUSE_AVIF=OFF
            -DUSE_JPEGXL=OFF
            -DUSE_LCMS=OFF
            -DUSE_WOFF2=OFF
            -DENABLE_IMAGE_FORMATS=OFF

            # Disable networking
            -DENABLE_NETWORKING=OFF
            -DENABLE_TEXT_RENDERING=OFF
            -DENABLE_XML_SUPPORT=OFF
            -DENABLE_DATABASE=OFF
            -DENABLE_THEMING=OFF

            # Disable features that require external dependencies or are browser-only
            -DENABLE_VIDEO=OFF
            -DENABLE_WEB_AUDIO=OFF
            -DENABLE_SAMPLING_PROFILER=OFF
            -DENABLE_WEBASSEMBLY=OFF
            -DENABLE_WEB_RTC=OFF
            -DENABLE_MEDIA_STREAM=OFF
            -DENABLE_GAMEPAD=OFF
            -DENABLE_GEOLOCATION=OFF
            -DENABLE_ENCRYPTED_MEDIA=OFF

            # JIT configuration (enable what's supported/stable)
            -DENABLE_JIT=ON
            -DENABLE_YARR_JIT=ON
            -DENABLE_CONCURRENT_JIT=OFF
            -DENABLE_DFG_JIT=ON
            -DENABLE_FTL_JIT=OFF

            # Keep minimal debugging support
            -DENABLE_REMOTE_INSPECTOR=${ENABLE_REMOTE_INSPECTOR}
            -DDEVELOPER_MODE=${DEVELOPER_MODE}

            ${WEBKIT_CLANG_ARGS}

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
                pthread dl m atomic
        )
    endif()

    add_library(JavaScriptCore_Complete INTERFACE)
    target_link_libraries(JavaScriptCore_Complete INTERFACE
            JavaScriptCore
            JavaScriptCore_Dependencies
    )

    message(STATUS "JSC Include directory: ${JSC_INCLUDE_DIR}")
endfunction()