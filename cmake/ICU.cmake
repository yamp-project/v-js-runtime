if(WIN32)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
        set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-77-1/icu4c-77_1-WinARM64-MSVC2022.zip")
        set(ICU_HASH "SHA512=d05b0bbe036ece10444758a200603ec1a0e5a635e798dcde91ae5a1bf2572a88fb8a209cd86352f3cf6f72ef0f5910b728ebb497017587e1d69a14c0e050bd6a")
    else()
        set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-77-1/icu4c-77_1-Win64-MSVC2022.zip")
        set(ICU_HASH "SHA512=b88b1013b2de56ad5f61a6f31997046851e882e5373579e565f32f455a44f4b9e814197129a3547f600eac184d98075b7ccaefcc46ee80a5c7e3a7a0fcd3e08a")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(EXISTS "/etc/fedora-release" OR EXISTS "/etc/redhat-release")
        set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-77-1/icu4c-77_1-Fedora_Linux40-x64.tgz")
        set(ICU_HASH "SHA512=2e0f919cebae72d9010bfa9c4edb418ffea5a6db2bc0ef5b1c906b4d404c5294244eeaf8b5401a8005b29ab5deed2b5637aa18f65a1e7a30549853c896aca195")
    else()
        set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-77-1/icu4c-77_1-Ubuntu22.04-x64.tgz")
        set(ICU_HASH "SHA512=e9f0f0cbbad9e328898bfabd38af7f461cc140dbae5d03c8e1f37b04303ccf52710263cf624f3d5e83f69c43ae98a991ef5ebb19a98dc535023c2b740835b2f8")
    endif()
else()
    message(FATAL_ERROR "Unsupported platform for prebuilt ICU")
endif()

ExternalProject_Add(ICU_External
        URL ${ICU_URL}
        URL_HASH ${ICU_HASH}

        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""

        INSTALL_COMMAND
        ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include <INSTALL_DIR>/include
        COMMAND $<IF:$<BOOL:${WIN32}>,
        ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib64 <INSTALL_DIR>/lib,
        ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib <INSTALL_DIR>/lib
        >

        LOG_DOWNLOAD ON
        LOG_INSTALL ON
)

set(ICU_EXTERNAL_DIR "${CMAKE_BINARY_DIR}/ICU_External-prefix")
set(ICU_ROOT "${ICU_EXTERNAL_DIR}")
set(ICU_INCLUDE_DIR "${ICU_EXTERNAL_DIR}/include")
set(ICU_LIBRARY_DIR "${ICU_EXTERNAL_DIR}/lib")

add_library(ICU::uc STATIC IMPORTED)
add_library(ICU::i18n STATIC IMPORTED)
add_library(ICU::data STATIC IMPORTED)

if(WIN32)
    set_target_properties(ICU::uc PROPERTIES
            IMPORTED_LOCATION "${ICU_LIBRARY_DIR}/icuuc.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
    )

    set_target_properties(ICU::i18n PROPERTIES
            IMPORTED_LOCATION "${ICU_LIBRARY_DIR}/icuin.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES ICU::uc
    )

    set_target_properties(ICU::data PROPERTIES
            IMPORTED_LOCATION "${ICU_LIBRARY_DIR}/icudt.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
    )
else()
    set_target_properties(ICU::uc PROPERTIES
            IMPORTED_LOCATION "${ICU_LIBRARY_DIR}/libicuuc.a"
            INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
    )

    set_target_properties(ICU::i18n PROPERTIES
            IMPORTED_LOCATION "${ICU_LIBRARY_DIR}/libicuin.a"
            INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES ICU::uc
    )

    set_target_properties(ICU::data PROPERTIES
            IMPORTED_LOCATION "${ICU_LIBRARY_DIR}/libicudt.a"
            INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
    )
endif()

file(MAKE_DIRECTORY "${ICU_INCLUDE_DIR}")
file(MAKE_DIRECTORY "${ICU_LIBRARY_DIR}")

add_dependencies(ICU::uc ICU_External)
add_dependencies(ICU::i18n ICU_External)
add_dependencies(ICU::data ICU_External)