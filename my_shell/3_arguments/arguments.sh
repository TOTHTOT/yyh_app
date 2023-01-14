#!/bin/bash
###
 # @Description: shell 参数
 # @Author: TOTHTOT
 # @Date: 2023-01-13 14:41:33
 # @LastEditTime: 2023-01-13 15:18:40
 # @LastEditors: TOTHTOT
 # @FilePath: \my_shell\3_arguments\arguments.sh
### 

echo "Shell 参数例程"
echo "File Name:$0"
echo "Name:$1 Age:$2, 当前进程号:$$"
echo "总共传入$#个参数"

echo "测试 \$*"
for i in "$*";
do
    echo ${i}
done

echo "测试 \$@ "
for i in "$@";
do
    echo ${i}
done







