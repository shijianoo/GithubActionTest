# ==============================
# generated_files.cmake
# 生成安装程序/卸载程序需要的资源和配置文件
# 并检查生成是否成功
# ==============================

# --- 1. 配置头文件 config.h ---
set(CONFIG_H "${CMAKE_CURRENT_LIST_DIR}/../inc/config.h")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/../inc/config.h.in"
    ${CONFIG_H}
    @ONLY
)

if(NOT EXISTS ${CONFIG_H})
    message(FATAL_ERROR "生成配置文件失败: ${CONFIG_H} 不存在")
endif()

# --- 2. 安装程序通用资源文件 install.rc ---
set(INSTALL_RC "${CMAKE_CURRENT_LIST_DIR}/../src/install.rc")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/../src/install.rc.in"
    ${INSTALL_RC}
    @ONLY
)

if(NOT EXISTS ${INSTALL_RC})
    message(FATAL_ERROR "生成资源文件失败: ${INSTALL_RC} 不存在")
endif()

# --- 3. 在线运行时资源文件 online_runtime.rc ---
set(ONLINE_RUNTIME_RC "${CMAKE_CURRENT_LIST_DIR}/../src/online_runtime.rc")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/../src/online_runtime.rc.in"
    ${ONLINE_RUNTIME_RC}
    @ONLY
)

if(NOT EXISTS ${ONLINE_RUNTIME_RC})
    message(FATAL_ERROR "生成资源文件失败: ${ONLINE_RUNTIME_RC} 不存在")
endif()

# --- 4. 离线运行时资源文件 offline_runtime.rc ---
set(OFFLINE_RUNTIME_RC "${CMAKE_CURRENT_LIST_DIR}/../src/offline_runtime.rc")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/../src/offline_runtime.rc.in"
    ${OFFLINE_RUNTIME_RC}
    @ONLY
)

if(NOT EXISTS ${OFFLINE_RUNTIME_RC})
    message(FATAL_ERROR "生成资源文件失败: ${OFFLINE_RUNTIME_RC} 不存在")
endif()