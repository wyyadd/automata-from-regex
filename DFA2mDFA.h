//
// Created by wyyadd on 12/15/21.
//

#ifndef COMPILE_DFA2mDFA_H
#define COMPILE_DFA2mDFA_H

#include <utility>
#include <vector>
#include <set>
#include <memory>
#include "NFA2DFA.h"

namespace Lexical {
    using std::vector;
    using std::set;
    using std::stack;
    using std::unique_ptr;
    using std::shared_ptr;

    struct mDFA {
        int state_;
        bool dead = true;
        bool infiniteLoop = false;
        bool acceptable = false;
        vector<std::pair<char, mDFA *>> links_;
    };

    struct Group {
        set<int> member_;
    };

    class DFA2mDFA {
    public:
        explicit DFA2mDFA(vector<shared_ptr<DFA *>> DFA, set<char> charSet);

        void GenerateGroup();

        // group g的每一个dfa，move(dfa, c)不属于g， 则认为该dfa不在g组
        // ture means same group
        // false means different group
        bool DivideGroup(Group *g, char c, stack<Group *> &s);

        void Generate_mDFA();

        // 判断dfa状态所属的group
        int getGroupState(int s);

        // delete mDFA which cannot be reached from entry
        void ReduceRedundancy(mDFA *start);

        // mark mDFA which is not exit node and its links always point to itself
        void CacheInfiniteLoop();

        void Show_mDFA();

        // 判断字符串是否满足正则规则
        bool Check(const std::string &s);

        bool walkThroughDFA(const std::string &s, int po, mDFA *m_dfa);


    private:
        const vector<shared_ptr<DFA *>> DFA_;
        const set<char> charSet_;
        vector<unique_ptr<Group *>> Group_;
        vector<unique_ptr<mDFA *>> mDFA_;
        mDFA *entry = nullptr;
        set<mDFA *> exit;
    };
}
#endif //COMPILE_DFA2mDFA_H
