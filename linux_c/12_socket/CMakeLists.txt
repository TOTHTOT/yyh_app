# CMake 最低版本要求
cmake_minimum_required(VERSION 3.5)

# 项目名称
project(MyProject)

# 交叉编译工具链路径
set(CMAKE_C_COMPILER /usr/bin/gcc)

# 添加可执行目标

# 定义源文件
set(SOURCE_FILES "")

# 根据传入的参数选择源文件
if(ARG STREQUAL "1")
    add_executable(socket_tcp_server)
    list(APPEND SOURCE_FILES "socket_tcp_server.c")
    target_sources(socket_tcp_server PRIVATE ${SOURCE_FILES})

elseif(ARG STREQUAL "2")
    add_executable(socket_tcp_server)
    list(APPEND SOURCE_FILES "2.c")
else()
    message(FATAL_ERROR "Invalid argument. Use 1 or 2.")
endif()

# 将源文件添加到可执行目标
