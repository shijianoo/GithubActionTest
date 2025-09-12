# check_resources.cmake
# 用于检查打包所需文件是否存在

# 检查发布目录是否存在
if(NOT EXISTS "${SOFT_RELEASE}")
    message(FATAL_ERROR "软件发布目录 ${SOFT_RELEASE} 不存在")
endif()

# 检查主程序 exe 是否存在
if(NOT EXISTS "${APP_EXE_PATH}")
    message(FATAL_ERROR "可执行程序 ${APP_EXE_PATH} 不存在")
endif()

# 检查卸载程序是否存在
if(NOT EXISTS "${UNINSTALL_PATH}")
    message(FATAL_ERROR "卸载程序 ${UNINSTALL_PATH} 不存在")
endif()

# 检查数据文件图标是否存在
if(NOT EXISTS "${FILEICON_PATH}")
    message(FATAL_ERROR "SSTD 数据文件图标 ${FILEICON_PATH} 不存在")
endif()

# 在线运行时
if(NOT EXISTS "${ONLINE_RUNTIME_PATH}")
     message(STATUS "在线运行时不存在，开始从微软官网下载...")
     file(DOWNLOAD
        "https://go.microsoft.com/fwlink/?LinkId=2085155"
        "${ONLINE_RUNTIME_PATH}"
        SHOW_PROGRESS
        STATUS _dl_status
        LOG _dl_log
    )
    list(GET _dl_status 0 _dl_code)
    if(NOT _dl_code EQUAL 0)
        message(WARNING "下载在线运行时失败: ${_dl_log}")
        message(FATAL_ERROR "请手动从 https://go.microsoft.com/fwlink/?LinkId=2085155 下载 ndp48-web.exe 并放到 res 目录下")
    endif()
endif()

# 离线运行时
if(NOT EXISTS "${OFFLINE_RUNTIME_PATH}")
     message(STATUS "离线运行时不存在，开始从微软官网下载...")
     file(DOWNLOAD
        "https://go.microsoft.com/fwlink/?LinkId=2088631"
        "${OFFLINE_RUNTIME_PATH}"
        SHOW_PROGRESS
        STATUS _dl_status
        LOG _dl_log
    )
    list(GET _dl_status 0 _dl_code)
    if(NOT _dl_code EQUAL 0)
        message(WARNING "下载离线运行时失败: ${_dl_log}")
        message(FATAL_ERROR "请手动从 https://go.microsoft.com/fwlink/?LinkId=2088631 下载 NDP48-x86-x64-AllOS-ENU.exe 并放到 res 目录下")
    endif()
endif()