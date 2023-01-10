cmake_minimum_required (VERSION 2.8)
# 设置交叉编译器
set (CMAKE_C_COMPILER "/home/yyh/ti-sdk-am335x-evm-06.00.00.00/linux-devkit/sysroots/i686-arago-linux/usr/bin/arm-linux-gnueabihf-gcc")
project(serial_thread)

add_executable(serial_thread main.c uart.c)
#添加 pthread 库
target_link_libraries(serial_thread pthread)
