cmake_minimum_required(VERSION 3.29.0)
set(CMAKE_CXX_STANDARD 20)
project(Instaler)

set(SOFT_NAME "" CACHE STRING "软件名称 (必填)")
set(SOFT_VERSION "" CACHE STRING "软件版本 (必填)")
set(SOFT_RELEASE_DIR "" CACHE STRING "软件发布目录 (必填)")

# 检查用户是否传递
foreach(var SOFT_NAME SOFT_VERSION SOFT_RELEASE_DIR)
    if("${${var}}" STREQUAL "")
        message(FATAL_ERROR "必须指定 ${var}，例如: -D${var}=VALUE")
    endif()
endforeach()

# ---------------- 基本信息 ----------------
set(APP_EXE_PATH "${SOFT_RELEASE_DIR}/${SOFT_NAME}.exe") #可执行文件路径
set(UNINSTALL_PATH "${CMAKE_CURRENT_LIST_DIR}/res/Uninstaller.exe") #卸载应用程序路径
set(FILEICON_PATH "${CMAKE_CURRENT_LIST_DIR}/res/sstdfile.ico") #SSTD数据文件图标路径
set(ONLINE_RUNTIME_PATH "${CMAKE_CURRENT_LIST_DIR}/res/ndp48-web.exe") #在线运行时路径
set(OFFLINE_RUNTIME_PATH "${CMAKE_CURRENT_LIST_DIR}/res/ndp48-x86-x64-allos-enu.exe") #离线运行时路径
set(APP_ZIP "${CMAKE_CURRENT_LIST_DIR}/res/app.zip") #应用程序打包文件路径
set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../bin/${SOFT_VERSION}")

# ---------------- 模块路径 ----------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(check_resources) #确保资源检查已完成
include(package_resources) #执行打包资源到 app.zip 的操作
include(generated_files) #构建压缩文件和资源文件的路径

# ---------------- 打印信息 ----------------
message("软件名称: ${SOFT_NAME}")
message("软件版本: ${SOFT_VERSION}")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_subdirectory(../3rd/duilib ${CMAKE_CURRENT_BINARY_DIR}/duilib_build)
add_subdirectory(../common ${CMAKE_CURRENT_BINARY_DIR}/common_build)

# ---------------- 源文件 ----------------
list(APPEND INSTALL_SOURCE_FILES 
    src/main.cpp
    ${INSTALL_RC}
    src/InstallWindow.cpp
    src/InstallHandler.cpp
    src/icoextra.cpp
    src/DotNetInstallServer.cpp
)

# ---------------- INTERFACE 库管理公共设置 ----------------
add_library(common_settings INTERFACE)
target_include_directories(common_settings INTERFACE ${CMAKE_CURRENT_LIST_DIR}/inc)
target_link_libraries(common_settings INTERFACE common DuiLib)
target_compile_definitions(common_settings INTERFACE UILIB_STATIC ZLIB_WINAPI)

# ---------------- 生成在线运行时安装程序 ----------------
set(INSTALL_NAME_ONLINE "${SOFT_NAME}_${SOFT_VERSION}_setup")
add_executable(${INSTALL_NAME_ONLINE} WIN32 ${INSTALL_SOURCE_FILES} ${ONLINE_RUNTIME_RC})
target_link_libraries(${INSTALL_NAME_ONLINE} common_settings)
set_target_properties(${INSTALL_NAME_ONLINE} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
    LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
)

# ---------------- 生成脱机运行时安装程序 ----------------
set(INSTALL_NAME_OFFLINE "${SOFT_NAME}_${SOFT_VERSION}_setup_offline")
add_executable(${INSTALL_NAME_OFFLINE} WIN32 ${INSTALL_SOURCE_FILES} ${OFFLINE_RUNTIME_RC})
target_link_libraries(${INSTALL_NAME_OFFLINE} common_settings)
set_target_properties(${INSTALL_NAME_OFFLINE} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
    LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
)

# ---------------- 清理资源 ----------------
add_custom_target(cleanup_resources ALL
    COMMAND ${CMAKE_COMMAND} -E remove -f
        "${UNINSTALL_PATH}"
        "${ONLINE_RUNTIME_PATH}"
        "${OFFLINE_RUNTIME_PATH}"
        "${APP_ZIP}"
        "${CONFIG_H}"
        "${INSTALL_RC}"
        "${ONLINE_RUNTIME_RC}"
        "${OFFLINE_RUNTIME_RC}"
    COMMENT "删除临时文件"
)

# 确保 cleanup 在两个可执行文件生成后再执行
add_dependencies(cleanup_resources
    ${INSTALL_NAME_ONLINE}
    ${INSTALL_NAME_OFFLINE}
)
