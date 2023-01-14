#!/bin/bash
###
# @Description: shell 之 test 命令
# @Author: TOTHTOT
# @Date: 2023-01-13 17:27:32
# @LastEditTime: 2023-01-13 17:38:10
# @LastEditors: TOTHTOT
# @FilePath: \my_shell\7_test\test.sh
###

argv=($1 $2)

#测试两个变量是否等于指定值, -a为"于" -o为"或", !为"非"
if test ${argv[0]} = MD -a ${argv[1]} = SD; then
    echo cmd success ${argv[0]} ${argv[1]}
else
    echo cmd fail ${argv[0]} ${argv[1]}
fi
