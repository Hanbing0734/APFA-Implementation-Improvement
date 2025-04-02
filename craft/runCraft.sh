#!/bin/bash

###
 # @Author: Hanbing
 # @Date: 2024-09-25 11:55:37
 # @LastEditors: Hanbing
 # @LastEditTime: 2024-10-29 08:56:45
 # @FilePath: /SAT/craft/run.sh
 # @Description: 
 # 
 # Copyright (c) 2024 by Hanbing, All Rights Reserved. 
### 

runC(){

    parentDir="./IO/out/${my_program}_noTimeLimit"
    if [ ! -d "$parentDir" ];then
        mkdir -p $parentDir
        # echo "mkdir $parentDir"
    fi
    # parentDir2="./IO/out/com/${1}_${2}_${3}"
    # if [ ! -d "$parentDir2" ];then
    #     mkdir -p $parentDir2
    #     # echo "mkdir $parentDir2"
    # fi

    local output_file="${parentDir}/${1}_${2}_${3}_$(date "+%Y_%m_%d_%H:%M:%S").out"
    # local output_file2="${parentDir2}/${my_program}_$(date "+%Y_%m_%d_%H:%M:%S").out"

    nohup time -v ./bin/$my_program $1 $2 $3 $4 > "$output_file" 2>&1 &
    # nohup time -v ./bin/$my_program $1 $2 $3 $4 > "$output_file" 2>&1 && cp "$output_file" "$output_file2" &
    wait $! # 等待后台进程结束
}
# 运行不同文件
run(){
    # 文件名
    my_program=$1
    # 循环次数
    # 分析轮数      约束
    numAnalysesRoundsStart=$2
    numAnalysesRoundsEnd=$3
    numAnalysesStep=1
    # 使用的最少错误密文数
    numMinCipher=$4
    numMaxCipher=$5

    # 主体 循环多次收集平均数据
    for ((i = 0; i < $loops; i++))
    do
        # 开始分析的起始轮
        for ((numAnalysesRounds=$numAnalysesRoundsStart; numAnalysesRounds<=$numAnalysesRoundsEnd; numAnalysesRounds+=$numAnalysesStep))
        do
            runC $Threads $numAnalysesRounds $numMinCipher $numMaxCipher
        done
    done
}

# | program_name | AnalysesRoundsSt | AnalysesRoundsEn | minMinCipher | maxMinCipher

Threads=1

loops=1

# run "craft"  5 12 60 60 &
# run "craft_NS"  5 12 60 60 &
run "craft"  12 12 80 80 &
# run "craft_NS"  5 12 80 80 &

# run "craft_NS"  6 6 50 80 
# run "craft_NS"  7 7 47 70  

# run "craft"  6 6 50 80 
# run "craft"  7 7 47 70 

# run "craft"  7 12 45 60 &
# run "craft_NS"  7 12 45 60 &

# run "craft"  5 12 55 70 &
# run "craft_NS"  5 6 55 70 

# run "craft"  8 12 55 80 &

