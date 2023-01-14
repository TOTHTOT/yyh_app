#!/bin/bash
###
 # @Description: shell之基本运算符
 # @Author: TOTHTOT
 # @Date: 2023-01-13 17:08:43
 # @LastEditTime: 2023-01-13 17:19:21
 # @LastEditors: TOTHTOT
 # @FilePath: \my_shell\6_operator\operator.sh
### 
a=$1
b=$2
if [ $# == 0 ]
then
    echo 参数太少了
fi

val=`expr $a + $b`
printf "a + b = %d\n" ${val}

val=`expr $a - $b`
printf "a - b = %d\n" ${val}

val=`expr $a \* $b`
printf "a * b = %d\n" ${val}

val=`expr $a / $b`
printf "a / b = %d\n" ${val}

val=`expr $a % $b`
printf "a %% b = %d\n" ${val}

if [ $a == $b ]
then
    printf "a == b\n"
fi

if [ $a != $b ]
then
    printf "a != b\n"
fi