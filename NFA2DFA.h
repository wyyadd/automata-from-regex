//
// Created by wyyadd on 12/13/21.
//

#ifndef COMPILE_NFA2DFA_H
#define COMPILE_NFA2DFA_H

#include <utility>
#include <vector>
#include <unordered_map>
#include <set>
#include <memory>
#include "Re2NFA.h"


namespace Lexical {
    using std::vector;
    using std::set;
    using std::unique_ptr;
    using std::shared_ptr;
    using std::pair;

    struct DFA {
        explicit DFA(int state) {
            state_ = state;
        }

        int state_;
        bool acceptable = false;
        vector<pair<char, int>> links_;
    };

    struct D_state {
        D_state(set<int> NFAStates, int state) {
            NFAStates_ = std::move(NFAStates);
            state_ = state;
        }

        set<int> NFAStates_;
        int state_;
    };

    class NFA2DFA {
    public:
        explicit NFA2DFA(vector<shared_ptr<NFA *>> nfa, set<char> charSet);

        void GenerateDFA();

        // 求单个nfa节点的epsilon闭包
        set<int> e_closure(NFA *nfa);

        // 求nfa集合的闭包
        set<int> e_closure(const vector<NFA *> &T);

        // 求该nfa集合通过字符c转移到的nfa集合
        vector<NFA *> Move(const set<int> &T, char c);

        // 求该nfa集合对应的dfa状态
        int Get_DStates(const set<int> &states);

        void ShowDFA();

        vector<shared_ptr<DFA *>> &getDFA();

    private:
        const vector<shared_ptr<NFA *>> nfa_;
        const set<char> charSet_;
        vector<unique_ptr<D_state *>> D_states_;
        vector<shared_ptr<DFA *>> D_tran_;
    };

}

#endif //COMPILE_NFA2DFA_H
