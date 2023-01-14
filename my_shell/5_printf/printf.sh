#!/bin/bash
###
 # @Description: shell之printf
 # @Author: TOTHTOT
 # @Date: 2023-01-13 15:50:17
 # @LastEditTime: 2023-01-13 15:55:47
 # @LastEditors: TOTHTOT
 # @FilePath: \my_shell\5_printf\printf.sh
### 
name="杨逸辉"
age=23
array=(杨逸辉 23)

printf "直接输出: Name:%s, Age:%d\n" 杨逸辉 23
printf "使用变量: Name:%s, Age:%d\n" ${name} ${age}
printf "使用数组: Name:%s, Age:%d\n" ${array[0]} ${array[1]}
