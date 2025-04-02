#!/bin/bash
###
 # @Author: Hanbing
 # @Date: 2024-09-30 16:33:11
 # @LastEditors: Hanbing
 # @LastEditTime: 2024-10-12 09:04:54
 # @FilePath: /SAT/CRAFT/kill.sh
 # @Description: 
 # 
 # Copyright (c) 2024 by Hanbing, All Rights Reserved. 
### 

# if [ -f runPresents.pid ]; then
#     while read -r PID; do
#         kill "$PID" 2>/dev/null
#     done < runPresents.pid
#     pkill -u hanbing -f present

#     # 删除 PID 文件以避免之后的混淆
#     rm runPresents.pid
# else
#     echo "No PID file found."
# fi


pkill -u hanbing -f runCraft.sh
pkill -u hanbing -f craft