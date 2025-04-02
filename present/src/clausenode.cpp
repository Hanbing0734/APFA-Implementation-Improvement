/*
 * @Author: Hanbing
 * @Date: 2024-09-14 07:52:59
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-09-26 09:23:10
 * @FilePath: /SAT/present/include/clausenode.cpp
 * @Description:
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */

#include "clausenode.h"

unsigned int ClauseNode::getIndex() { return index; }
void ClauseNode::setIndex(unsigned int index_) { index = index_; }

bool ClauseNode::usedNode() { return used_node; }
void ClauseNode::setUsedNode(bool usedNode) { used_node = usedNode; }
int ClauseNode::newClauseNode(CMSat::SATSolver &solver, bool check_used) {
    if (check_used == NODE_CHECK_USED) {
        if (!this->usedNode()) {
            this->setUsedNode(true);
            this->setIndex((unsigned int)solver.nVars());
            solver.new_var();
        } else {
            return this->getIndex();
        }
    } else {
        this->setUsedNode(true);
        this->setIndex((unsigned int)solver.nVars());
        solver.new_var();
    }
    return this->getIndex();
}
