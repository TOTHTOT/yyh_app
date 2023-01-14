#!/bin/bash

#字符串
my_str="hello world!"
#数组
my_array_country=("China" "Russia" "America" "England")

# 设置只读
readonly my_str

echo I said ${my_str} in shell. The string len:${#my_str}.
echo I am form ${my_array_country[0]}.

#删除变量
#unset my_str
