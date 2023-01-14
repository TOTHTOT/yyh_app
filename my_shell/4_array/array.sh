#!/bin/bash
###
 # @Description: 关联数组
 # @Author: TOTHTOT
 # @Date: 2023-01-13 15:27:03
 # @LastEditTime: 2023-01-13 15:34:30
 # @LastEditors: TOTHTOT
 # @FilePath: \my_shell\4_array\array.sh
### 

#创建一个关联数组
declare -A site
site["google"]="www.google.com"
site["baidu"]="www.baidu.com"
site["taobao"]="www.taobao.com"

#输出指定键值对应的数据
echo "输出指定键值内容:google:${site["google"]}"
#输出数组里的所有元素, 有两种方式
echo "所有内容: ${site[*]}"
echo "所有内容: ${site[@]}"
#获取数组里的元素个数
echo "有${#site[*]}个元素"
echo "有${#site[@]}个元素"