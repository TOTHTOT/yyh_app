cmake_minimum_required (VERSION 2.8)
# 设置交叉编译器
set (CMAKE_C_COMPILER "/home/yyh/ti-sdk-am335x-evm-06.00.00.00/linux-devkit/sysroots/i686-arago-linux/usr/bin/arm-linux-gnueabihf-gcc")
# 设置内核路径
set (KDIR /home/yyh/Work/to_share/linux_kernel)
project(hello_world)
# 定义编译函数/方法，驱动编译过程主要在这个函数进行
function(compile_module obj)
  set(TARGET_NAME ${obj})
  # 添加目标，obj即为模块名称。由于我在build目录下构建，所以这里选择将.c和.h文件拷贝到该目录下
  add_custom_target(${TARGET_NAME} ALL cp -f ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_BINARY_DIR}/
			# COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_BINARY_DIR}/
  			COMMAND echo "compiling module ${obj}.ko..."
  		    )

  # 设置依赖，相当于Makefile中的 $(MODULE_NAME)-objs += hello_world.o
  set(depend_objlist "hello_world.o")
  
  # 设置编译命令
  add_custom_command(TARGET ${TARGET_NAME}
		     POST_BUILD
		     COMMAND echo "obj-m := ${obj}.o" > ${CMAKE_CURRENT_BINARY_DIR}/Makefile
		     COMMAND echo "${obj}-objs:=${depend_objlist}" >>${CMAKE_CURRENT_BINARY_DIR}/Makefile
		     COMMAND make -C ${KDIR} M=${CMAKE_CURRENT_BINARY_DIR} modules
		     )
endfunction()

# 调用编译函数
compile_module(hello_world)
