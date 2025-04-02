/*
 * @Author: Hanbing
 * @Date: 2024-09-14 07:48:39
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-11 16:17:11
 * @FilePath: /SAT/4_CRAFT/include/clausenode.h
 * @Description: 将加密状态转换为cryptominisat需要的格式对应的编号
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */

#ifndef _CRAFT_SRC_CLAUSENODE_H
#define _CRAFT_SRC_CLAUSENODE_H

#include <cryptominisat5/cryptominisat.h>
// 全局参数
#include "params.h"

class ClauseNode {
private:
    unsigned int index;
    bool used_node = false;

public:
    unsigned int getIndex() const;
    void setIndex(unsigned int);
    bool usedNode() const;
    void setUsedNode(bool);
    /**
     * @description: 若check_used == true，则检查，如果该节点已使用，则返回该节点index，否则申请新节点返回。
     *               若check_used == false，则不检查，直接申请新节点返回
     * @param {SATSolver&} solver
     * @param {int} check_used
     * @return {*}
     */
    int newClauseNode(CMSat::SATSolver &solver, int check_used);
    // NODE_NOT_CHECK_UESD
    int newNode(CMSat::SATSolver &solver);
};

#endif  // _CRAFT_SRC_CLAUSENODE_H