#include <assert.h>
#include <cryptominisat5/cryptominisat.h>

#include <algorithm>  //C++一些常用函数库
#include <cctype>
#include <cstring>   //C++扩展String字符串流
#include <iostream>  //C++输入输出流
#include <vector>
using std::vector;
using namespace CMSat;
using namespace std;

int main() {
    SATSolver solver;
    vector<Lit> clause;
    vector<unsigned> xor_clause;
    int vars = 4;
    int threads = 4;

    // Let'd use 4 threads
    //  这一步可以不写？？？注释了也没有错
    solver.set_num_threads(threads);

    // We need 3 variables. They will be: 0,1,2
    // Variable numbers are always trivially increasing
    solver.new_vars(vars);

    // add "1 0"
    clause.push_back(Lit(0, false));
    solver.add_clause(clause);

    // add "-2 0"
    clause.clear();
    clause.push_back(Lit(1, true));
    solver.add_clause(clause);

    // add "-1 2 3 0"
    clause.clear();
    clause.push_back(Lit(0, true));
    clause.push_back(Lit(1, false));
    clause.push_back(Lit(2, false));
    solver.add_clause(clause);

    // 偶数个0(l_True)是false，奇数个0(l_True)是true
    // add x1 2 3 4 0
    xor_clause.push_back(0);
    xor_clause.push_back(1);
    xor_clause.push_back(2);
    xor_clause.push_back(3);
    solver.add_xor_clause(xor_clause, false);

    lbool ret = solver.solve();
    // l_True为0    用+xxx.getVa;ue()转化为01输出
    // std::cout<<"ret.getValue():\t"<<+ret.getValue()<<std::endl;
    assert(ret == l_True);
    std::cout
        << "Solution is: "
        << solver.get_model()[0]
        << ", " << solver.get_model()[1]
        << ", " << solver.get_model()[2]
        << ", " << solver.get_model()[3]
        // << ", " << solver.get_model()[4]
        // << ", " << solver.get_model()[5]
        << std::endl;
    int d[10];
    string s;
    for (int i = 0; i < vars; i++) {
        d[i] = +solver.get_model()[i].getValue();
        s += '0' + solver.get_model()[i].getValue();

        cout << +solver.get_model()[i].getValue() << endl;
        // cout<<d[i]<<endl;
    }
    cout << s << endl;
    /*
        Solution is: l_True, l_False, l_True, l_False
        0
        1
        0
        1
        0101
    */

    // assumes 3 = FALSE, no solutions left
    vector<Lit> assumptions;
    assumptions.push_back(Lit(2, true));
    ret = solver.solve(&assumptions);
    assert(ret == l_False);

    // without assumptions we still have a solution
    ret = solver.solve();
    assert(ret == l_True);

    // add "-3 0"
    // No solutions left, UNSATISFIABLE returned
    clause.clear();
    clause.push_back(Lit(2, true));
    solver.add_clause(clause);
    ret = solver.solve();
    assert(ret == l_False);

    return 0;
}
