#!/bin/bash
#自己的编译脚本, 需要将本文件放在 emb_general_make.sh 同一级目录

# 进入编译环境
source ~/ti-sdk-am335x-evm-06.00.00.00/linux-devkit/environment-setup

function echoUsage() {
    echo ""
    echo "Usage: ./yyh_bianyi.sh clean_all"
    echo "       ./yyh_bianyi.sh clean_emb"
    echo "       ./yyh_bianyi.sh clean_com"
    echo "       ./yyh_bianyi.sh build_all"
    echo "       ./yyh_bianyi.sh build_emb"
    echo "       ./yyh_bianyi.sh build_com"
    echo "This document needs to be placed in the emb_ general_ Make.sh directory at the same level"
    echo ""
}

if [ "$#" == 0 ]; then
    echoUsage
    exit 1
fi

# 判断 emb_general_make.sh 是否存在
if [ ! -e emb_general_make.sh ]; then
    echo "emb_ general_make.sh is not exist"
    echoUsage
    exit 1
fi

#定义一些变量
BUILD_CLEAN_FLAG="$1"
EMB_GENERAL_MAKE_PATH="./emb_general_make.sh"

# 对第一个参数进行判断
if [[ ${BUILD_CLEAN_FLAG} != "clean_all" && ${BUILD_CLEAN_FLAG} != "clean_emb" && ${BUILD_CLEAN_FLAG} != "clean_com" && ${BUILD_CLEAN_FLAG} != "build_all" && ${BUILD_CLEAN_FLAG} != "build_com" && ${BUILD_CLEAN_FLAG} != "build_emb"  ]]; then
    echo "you input ${BUILD_CLEAN_FLAG}"
    echoUsage
    exit 1
fi

if [ ${BUILD_CLEAN_FLAG} == "clean_all" ]; then
    ./emb_general_make.sh communicate clean
    ./emb_general_make.sh emb_app clean
    
elif [ ${BUILD_CLEAN_FLAG} == "clean_emb" ]; then
    ./emb_general_make.sh emb_app clean
elif [ ${BUILD_CLEAN_FLAG} == "clean_com" ]; then
    ./emb_general_make.sh communicate clean
elif [ ${BUILD_CLEAN_FLAG} == "build_all" ]; then
    ./emb_general_make.sh communicate clean
    ./emb_general_make.sh emb_app clean
    ./emb_general_make.sh communicate MD
    ./emb_general_make.sh emb_app MD
    echo "build all finish copy emb to /home/yyh/Work/linux/nfs"
    cp emb_app/emb /home/yyh/Work/linux/nfs
elif [ ${BUILD_CLEAN_FLAG} == "build_emb" ]; then
    ./emb_general_make.sh emb_app clean
    ./emb_general_make.sh emb_app MD
    echo "build emb finish copy emb to /home/yyh/Work/linux/nfs"
    cp emb_app/emb /home/yyh/Work/linux/nfs
elif [ ${BUILD_CLEAN_FLAG} == "build_com"  ]; then
    ./emb_general_make.sh communicate clean
    ./emb_general_make.sh communicate MD
    echo "build communicate finish"
fi
