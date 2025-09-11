# package_resources.cmake
# 打包发布目录和必要文件到 app.zip

# 确保必要变量存在
if(NOT DEFINED SOFT_RELEASE_DIR)
    message(FATAL_ERROR "SOFT_RELEASE_DIR 未定义")
endif()

if(NOT DEFINED SOFT_NAME)
    message(FATAL_ERROR "SOFT_NAME 未定义")
endif()

if(NOT DEFINED UNINSTALL_PATH)
    message(FATAL_ERROR "UNINSTALL_PATH 未定义")
endif()

if(NOT DEFINED FILEICON_PATH)
    message(FATAL_ERROR "FILEICON_PATH 未定义")
endif()

find_program(SEVENZIP NAMES 7z 7z.exe)
if(NOT SEVENZIP)
    message(FATAL_ERROR "未找到 7z，请确认已安装并加入 PATH")
endif()

message(STATUS "找到 7z: ${SEVENZIP}")
message(STATUS "开始打包所有资源到 ${APP_ZIP}")

# 执行 7z 打包
message("打包目录: ${pacg}")
execute_process(
    COMMAND 7z a "${APP_ZIP}" "${SOFT_RELEASE_DIR}/*" "${UNINSTALL_PATH}" "${FILEICON_PATH}"
    WORKING_DIRECTORY "${SOFT_RELEASE_DIR}"
)

# 检查 zip 是否生成
if(NOT EXISTS "${APP_ZIP}")
    message(FATAL_ERROR "打包文件 ${APP_ZIP} 不存在，构建中止")
else()
    message(STATUS "打包完成: ${APP_ZIP}")
endif()