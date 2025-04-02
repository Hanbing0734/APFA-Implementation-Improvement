/*
 * @Author: Hanbing
 * @Date: 2024-09-14 07:48:39
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-11 16:25:18
 * @FilePath: /SAT/present/src/clausenode.h
 * @Description: 将加密状态转换为cryptominisat需要的格式对应的编号
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */

#ifndef _PRESENT_SRC_CLAUSENODE_H
#define _PRESENT_SRC_CLAUSENODE_H

#include <cryptominisat5/cryptominisat.h>
// 全局参数
#include "params.h"

class ClauseNode {
private:
    unsigned int index;
    bool used_node = false;

public:
    unsigned int getIndex();
    void setIndex(unsigned int);
    bool usedNode();
    void setUsedNode(bool);
    /**
     * @description: 若check_used == true，则检查，如果该节点已使用，则返回该节点index，否则申请新节点返回。
     *               若check_used == false，则不检查，直接申请新节点返回
     * @param {SATSolver&} solver
     * @param {bool} check_used
     * @return {*}
     */
    int newClauseNode(CMSat::SATSolver &solver, bool check_used);
};

#endif  // _PRESENT_SRC_CLAUSENODE_H