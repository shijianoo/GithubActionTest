cmake_minimum_required(VERSION 3.29.0)
set(CMAKE_CXX_STANDARD 20)
project(Common)

# 创建 common 静态库
add_library(Common STATIC
    Helper.cpp
    Window.cpp
)

# 添加 include 路径
target_include_directories(Common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接 duilib
target_link_libraries(Common PUBLIC DuiLib)