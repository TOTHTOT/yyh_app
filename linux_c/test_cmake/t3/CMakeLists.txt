cmake_minimum_required(VERSION 2.8)
project (Demo3)
aux_source_directory (./ DIR_SRCS)
# 添加 math 子目录
add_subdirectory (math)

add_executable(Demo3 ${DIR_SRCS})

# 添加链接库
target_link_libraries(Demo3 MathFunctions)
