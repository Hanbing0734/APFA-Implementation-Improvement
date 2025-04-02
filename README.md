<!--
 * @Author: Hanbing
 * @Date: 2024-08-23 06:38:17
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-11 16:51:04
 * @FilePath: /SAT/README.md
 * @Description:
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
-->

## 一、程序介绍

### 后缀

    `NULL`：S盒转换为ANF格式；S盒在轮密钥计算完毕后注入错误
    `-NS`：S盒使用Logic Friday 转换为CNF格式；S盒在轮密钥计算完毕后注入错误
    `-NSNK`：若密钥扩展过程中使用S盒：S盒使用Logic Friday 转换为CNF格式；生成轮密钥的过程中也使用错误S盒

### present 算法

    80      128

#### skinny 算法

    64/64       64/64*2     64/64*3
    128/128     128/128*2   128/128*3

#### craft 算法

    64/128

## 二、编译

    使用makefile工具进行编译。

## 三、运行

    使用makefile工具调用sh 批处理文件进行操作，每个算法根目录下有runXXX.sh文件，在内部设置运行时参数。

## 四、代码结构

    `clausenode.h`：变量操作
    `params.h`：部分运行时参数
    `XXXfunctions.h`：算法加密及APFA代数构造
    `utils.h`：部分算法参数，预设
    `testXXX.cpp`：主程序
