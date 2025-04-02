#!/bin/bash

###
 # @Author: Hanbing
 # @Date: 2024-09-25 11:55:37
 # @LastEditors: Hanbing
 # @LastEditTime: 2024-10-13 13:20:19
 # @FilePath: /SAT/skinny/run.sh
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
runSkinny(){
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

runSkinny_64_3(){
        
    # runSkinny "skinny_64_3"  7 7 130 200
    runSkinny "skinny_64_3"  8 8 115 200 
    runSkinny "skinny_64_3"  9 9 110 200 
    runSkinny "skinny_64_3"  10 10 100 200 
    runSkinny "skinny_64_3"  11 11 100 200 
    runSkinny "skinny_64_3"  12 12 100 200 

}


runSkinny_64_3_NS(){
        
    runSkinny "skinny_64_3_NS"  7 7 300 300
    
}


Threads=1

loops=10

# runSkinny_64_3 &
runSkinny_64_3_NS &

# runSkinny "skinny_64_1"  5 12 20 40 &
# runSkinny "skinny_64_1_NS"  5 12 20 40 &

# runSkinny "skinny_64_2"  7 12 120 120 &
# runSkinny "skinny_64_2_NS"  7 12 120 120 &

# runSkinny "skinny_64_2"  9 9 120 120
# runSkinny "skinny_64_2"  10 10 120 120 &
# runSkinny "skinny_64_2"  11 11 120 120 &
# runSkinny "skinny_64_2"  12 12 120 120 &


# runSkinny "skinny_64_3"  7 12 300 400 &
# runSkinny "skinny_64_3_NS"  7 12 300 400 &


# runSkinny "skinny_64_3_NS"  8 8 120 400 
# runSkinny "skinny_64_3_NS"  9 9 110 400 
# runSkinny "skinny_64_3_NS"  10 10 100 400 
# runSkinny "skinny_64_3_NS"  11 11 100 400 
# runSkinny "skinny_64_3_NS"  12 12 100 400 


# runSkinny "skinny_64_3"  8 9 150 200 &
# runSkinny "skinny_64_3"  10 12 150 200 &
