/*
 * @Author: Hanbing
 * @Date: 2024-09-24 14:24:50
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-24 08:51:44
 * @FilePath: /SAT/present/src/params.h
 * @Description: 通用参数
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */
#ifndef _PRESENT_SRC_PARAMS_H
#define _PRESENT_SRC_PARAMS_H

#define ROUNDS 32
#define BLOCK_SIZE 64
#define ELEMENT_SIZE 4
#define SBOX_ITEM_SIZE 4

#define NODE_CHECK_USED 1
#define NODE_NOT_CHECK_USED 0

#define NONE_SOLUTION 0
#define HAVE_SOLUTION 1
#define ONE_SOLUTION 2
#define MULTI_SOLUTION 3

#define ANF_MODE 101
#define CNF_MODE 102
#define NO_ERROR 0
#define HAVE_ERROR 1

#define TEST 201
#define RUN 202

#define SBOX_ERROR_INDEX 0     // 0~15
#define SBOX_ERROR_POSITION 0  // 0~3
#define SAT_MAX_TIME 3600

#define KEY_TEST "10001011101111001111001101111100101011001000011111000100001100111010100101100010010011110011111001000111011010001100111110111000"

#if PRESENT_MODE == 80
#define KEY_SIZE 80
#define MIN_CIPHERS 20
#define MAX_CIPHERS 1000
#define THREADS 4
#define MAX_RESULTS_LOG2 10

#elif PRESENT_MODE == 128
#define KEY_SIZE 128
#define MIN_CIPHERS 25
#define MAX_CIPHERS 1000
#define THREADS 4
#define MAX_RESULTS_LOG2 10
#endif

#if SAT_MODE == 0  // 初始方案，sbox使用ANF方式处理，KSR使用正确S盒，加密过程使用错误S盒
#define SBOX_MODE ANF_MODE
#define KSR_HAVEERROR NO_ERROR
#define ENCYPT_HAVEERROR HAVE_ERROR

#elif SAT_MODE == 1  // sbox使用CNF方式处理，KSR使用正确S盒，加密过程使用错误S盒
#define SBOX_MODE CNF_MODE
#define KSR_HAVEERROR NO_ERROR
#define ENCYPT_HAVEERROR HAVE_ERROR

#elif SAT_MODE == 2  // sbox使用CNF方式处理，KSR使用错误S盒，加密过程使用错误S盒
#define SBOX_MODE CNF_MODE
#define KSR_HAVEERROR HAVE_ERROR
#define ENCYPT_HAVEERROR HAVE_ERROR
#endif

#endif  // _PRESENT_SRC_PARAMS_H