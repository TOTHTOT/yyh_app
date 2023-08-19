#!/bin/bash
###
 # @Description: 
 # @Author: TOTHTOT
 # @Date: 2023-08-19 15:28:42
 # @LastEditTime: 2023-08-19 17:03:56
 # @LastEditors: TOTHTOT
 # @FilePath: \yyh_app\linux_driver\4_gpio_beep\my_build.sh
### 

INPUT_ARGS_NUM=$#
BUILD_NAME="buzzer.ko"
NFS_MODULE_PATH="/home/yyh/Learn/file_sys/nfs/lib/modules/5.4.31"
CROSS_COMPLIER_TYPE="arm-none-linux-gnueabihf-gcc"
APP_SOURCE_C=$1
BUILD_APP_NAME="buzzer_app"

# check input args number
if [ "$INPUT_ARGS_NUM" -lt 1 ]; then
	echo "please input app path!"
	exit 1
fi

make clean

make 
# 检查 make 命令的返回值
if [ $? -eq 0 ]; then
    echo "Make succeede"
	echo ""
	sudo cp $BUILD_NAME $NFS_MODULE_PATH
# build app
	echo "build app"
	echo ""
	$CROSS_COMPLIER_TYPE $APP_SOURCE_C -o $BUILD_APP_NAME
	sudo cp $BUILD_APP_NAME  $NFS_MODULE_PATH
	echo "clean project"
	echo ""
	make clean
else
    echo "Make failed\n"
	make clean
fi



