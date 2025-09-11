cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(DuiLib VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 收集所有源文件
file(GLOB DUILIB_CORE_SOURCES
    "*.cpp"
    "*.h"
)

file(GLOB DUILIB_CONTROL_SOURCES
    "Control/*.cpp"
    "Control/*.h"
)

file(GLOB DUILIB_CORE_DIR_SOURCES
    "Core/*.cpp"
    "Core/*.h"
)

file(GLOB DUILIB_LAYOUT_SOURCES
    "Layout/*.cpp"
    "Layout/*.h"
)

file(GLOB DUILIB_UTILS_SOURCES
    "Utils/*.cpp"
    "Utils/*.h"
)

# 合并所有源文件
set(DUILIB_ALL_SOURCES
    ${DUILIB_CORE_SOURCES}
    ${DUILIB_CONTROL_SOURCES}
    ${DUILIB_CORE_DIR_SOURCES}
    ${DUILIB_LAYOUT_SOURCES}
    ${DUILIB_UTILS_SOURCES}
)

# 创建静态库
add_library(DuiLib STATIC ${DUILIB_ALL_SOURCES})

# 设置目标属性
target_compile_definitions(DuiLib PUBLIC
    _MBCS
    UILIB_STATIC
    WIN32
    _WINDOWS
)

# 设置包含目录
target_include_directories(DuiLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Control
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Layout
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils
)

# 设置预编译头
target_precompile_headers(DuiLib PRIVATE StdAfx.h)

# 链接系统库
target_link_libraries(DuiLib PUBLIC
    comctl32
    gdiplus
    imm32
    ole32
    oleaut32
    uuid
    winmm
)

# 设置编译选项
target_compile_options(DuiLib PRIVATE
    /W3
    /wd4505
    /wd4251
    /wd4189
    /wd4121
    /wd4100
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Release>:/Ob2>
    $<$<CONFIG:Release>:/DNDEBUG>
    $<$<CONFIG:Debug>:/Od>
    $<$<CONFIG:Debug>:/D_DEBUG>
    $<$<CONFIG:Debug>:/Zi>
)