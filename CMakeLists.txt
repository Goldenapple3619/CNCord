cmake_minimum_required(VERSION 3.20)

project(cncord
    LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_REQUIRED_LIBRARIES m CACHE BOOL "" FORCE)

option(CN_BUILD_TESTS  "Build test executable" OFF)
option(CN_SANITIZE  "" OFF)

if(CN_BUILD_TESTS)
    enable_testing()
endif()

if(NOT DEFINED DIST_DIR)
    get_filename_component(CWD_NAME ${CMAKE_SOURCE_DIR} NAME)
    set(DIST_DIR "${CMAKE_SOURCE_DIR}/dist/${CWD_NAME}")
else()
    get_filename_component(DIST_DIR "${DIST_DIR}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")
endif()

if(NOT DEFINED TARGET_ARCH)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(TARGET_ARCH "i386")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(TARGET_ARCH "amd64")
    endif()
endif()

include(FetchContent)

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "Disable curl executable" FORCE)
set(CURL_STATICLIB OFF CACHE BOOL "Build curl as shared" FORCE)

if(WIN32)
    set(CURL_USE_SCHANNEL ON CACHE BOOL "Use native Schannel" FORCE)
elseif(APPLE)
    set(CURL_USE_SECURETRANSPORT ON CACHE BOOL "Use SecureTransport" FORCE)
else()
    set(CURL_USE_OPENSSL ON CACHE BOOL "Use OpenSSL" FORCE)
endif()

FetchContent_Declare(
  curl
  URL https://curl.se/download/curl-8.11.1.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(curl)

set(ENABLE_CJSON_TEST OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_AND_STATIC_LIBS OFF CACHE BOOL "" FORCE)
set(ENABLE_CUSTOM_COMPILER_FLAGS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  cjson
  GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
  GIT_TAG v1.7.18
)
FetchContent_MakeAvailable(cjson)

find_package(OpenSSL REQUIRED)

if(CN_SANITIZE)
    add_compile_options(
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-common
        -fstack-protector-all
        -fstack-check
        -g
    )
    add_link_options(-fsanitize=address,undefined)
endif()

if (NOT CN_BUILD_TESTS)
    add_compile_options(
        -ffile-prefix-map=${CMAKE_SOURCE_DIR}=.
    )
endif()

add_custom_target(cndist ALL)
add_dependencies(cndist cncord)

add_library(cncord SHARED
    src/client.c
    src/command.c
    src/error.c
    src/event.c
    src/gateway.c
    src/parsing.c

    src/common/buffer.c
    src/common/clock.c
    src/common/generic_vector.c
    src/common/log.c
    src/common/thread.c

    src/types/attachment.c
    src/types/channel.c
    src/types/emoji.c
    src/types/guild.c
    src/types/member.c
    src/types/message.c
    src/types/role.c
    src/types/user.c
    src/types/embed.c

    src/cache/base.c
    src/cache/channel.c
    src/cache/guild.c
    src/cache/message.c

    src/api/base.c
    src/api/interactions.c
    src/api/message.c
    src/api/moderation.c
    src/api/reactions.c
    src/api/typing.c
    src/api/commands.c
    src/api/guild.c
    src/api/channel.c

    src/event/channel.c
    src/event/client.c
    src/event/guild.c
    src/event/interactions.c
    src/event/invite.c
    src/event/member.c
    src/event/message.c
    src/event/reaction.c
    src/event/role.c
    src/event/scheduled_event.c
    src/event/stage.c
    src/event/user.c
    src/event/voice.c
)

if(CN_BUILD_TESTS)
    target_compile_options(cncord
        PRIVATE
            --coverage -O0 -g
    )
    target_link_options(cncord PRIVATE --coverage)
else()
    if(CN_SANITIZE)
        target_compile_options(cncord
            PRIVATE
                -Wall -Wextra -Wshadow -Werror -O0
        )
    else()
        target_compile_options(cncord
            PRIVATE
                -Wall -Wextra -Wshadow -Werror -O2
        )
    endif()
endif()

target_include_directories(cncord
    PUBLIC
        include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cncord
    PUBLIC
        m
        cjson
        CURL::libcurl
)

if(WIN32)
    target_link_libraries(cncord PRIVATE ws2_32)
else()
    target_link_libraries(cncord PRIVATE pthread)
endif()

target_include_directories(cncord
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC
        ${cjson_SOURCE_DIR}
)

foreach(lib libcurl cjson cncord)
    add_custom_command(TARGET cndist POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/lib"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${lib}>"
                "${DIST_DIR}/lib/"
        )
endforeach()

foreach(lib cncord)
    add_custom_command(TARGET cndist POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/include"
            COMMAND ${CMAKE_COMMAND} -E copy
                "${CMAKE_SOURCE_DIR}/include/*.h"
                "${DIST_DIR}/include/"
    )
endforeach()

if(UNIX AND NOT APPLE)
    find_program(PATCHELF_EXECUTABLE patchelf)
    if(PATCHELF_EXECUTABLE)
        add_custom_command(TARGET cndist POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                -DLIB_DIR=${DIST_DIR}/lib
                -DBIN_DIR=${DIST_DIR}/bin
                -P ${CMAKE_SOURCE_DIR}/toolchain/strip_soversion.cmake
        )
    else()
        message(WARNING "patchelf not found versioned .so files will be kept")
    endif()
elseif(APPLE)
    find_program(INSTALL_NAME_TOOL_EXECUTABLE install_name_tool)

    if(INSTALL_NAME_TOOL_EXECUTABLE)
        add_custom_command(TARGET cndist POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                -DLIB_DIR=${DIST_DIR}/lib
                -DBIN_DIR=${DIST_DIR}/bin
                -P ${CMAKE_SOURCE_DIR}/toolchain/strip_dylibversion.cmake
        )
    else()
        message(WARNING
            "install_name_tool not found versioned .dylib files will be kept"
        )
    endif()
endif()
