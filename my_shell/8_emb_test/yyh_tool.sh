#!/bin/sh
#用于将编译好的emb文件覆盖原来的emb
#杀死 这两个进程
killall Sita emb
#复制emb到目标地址
cp /mnt/emb /usr/local/bin/emb 
#执行脚本
/usr/local/rcS