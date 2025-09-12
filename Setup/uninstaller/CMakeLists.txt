cmake_minimum_required(VERSION 3.29.0)
set(CMAKE_CXX_STANDARD 20)
project(Uninstaller)

set(SOFT_NAME "" CACHE STRING "软件名称 (必填)")
if("${SOFT_NAME}" STREQUAL "")
    message(FATAL_ERROR "必须指定软件名称")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_subdirectory(../3rd/duilib ${CMAKE_CURRENT_BINARY_DIR}/duilib_build)
add_subdirectory(../common ${CMAKE_CURRENT_BINARY_DIR}/common_build)

list(APPEND UNINSTALL_SOURCE_FILES
    src/Main.cpp
    src/uninstall.rc
    src/UnInstallWindow.cpp
    src/RegistryHelper.cpp
    src/FileCollector.cpp
    src/FileDeleter.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${UNINSTALL_SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/inc)
target_link_libraries(${PROJECT_NAME} DuiLib Common)
target_compile_definitions(${PROJECT_NAME} PRIVATE UILIB_STATIC SOFT_NAME="${SOFT_NAME}")

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../bin"
    LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
)