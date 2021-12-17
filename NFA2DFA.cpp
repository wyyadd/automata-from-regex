//
// Created by wyyadd on 12/13/21.
//

#include "NFA2DFA.h"

#include <utility>

namespace Lexical {

    NFA2DFA::NFA2DFA(vector<shared_ptr<NFA *>> nfa, set<char> charSet) : nfa_(std::move(nfa)),
                                                                         charSet_(std::move(charSet)) {}

    void NFA2DFA::GenerateDFA() {
        // init
        auto S0 = e_closure(*nfa_[0]);
        int state = 0;
        D_states_.emplace_back(std::make_unique<D_state *>(new D_state(S0, state)));
        D_tran_.emplace_back(std::make_unique<DFA *>(new DFA(state++)));
        std::stack<int> stack;
        stack.push(0);
        // while there is an unmarked state T in D_states
        while (!stack.empty()) {
            auto T = *D_states_[stack.top()];
            // mark T = pop it
            stack.pop();
            // for each input symbol a
            for (char c: charSet_) {
                // U = e-closure(move(T,a))
                auto U = e_closure(Move(T->NFAStates_, c));
                if (U.empty())
                    continue;
                int U_state = Get_DStates(U);
                // if U is not in D_states
                if (U_state == -1) {
                    D_states_.emplace_back(std::make_unique<D_state *>(new D_state(U, state)));
                    D_tran_.emplace_back(std::make_unique<DFA *>(new DFA(state)));
                    stack.push(state);
                    U_state = state++;
                }
                // D_tran[T, a】 = U
                (*D_tran_[T->state_])->links_.emplace_back(std::make_pair(c, U_state));
            }
        }
        // set if acceptable
        auto acceptState = (int) nfa_.size() - 1;
        for (auto &i: D_states_) {
            if ((*i)->NFAStates_.contains(acceptState))
                (*D_tran_[(*i)->state_])->acceptable = true;
        }
    }

    std::set<int> NFA2DFA::e_closure(NFA *nfa) {
        std::stack<NFA *> stack;
        std::set<int> result;
        result.insert(nfa->state_);
        stack.push(nfa);
        while (!stack.empty()) {
            auto n = stack.top();
            stack.pop();
            for (auto &i: n->epsilon_) {
                if (!result.contains(i))
                    stack.push(*nfa_[i]);
                result.insert(i);
            }
        }
        return result;
    }

    std::set<int> NFA2DFA::e_closure(const std::vector<NFA *> &T) {
        std::set<int> result;
        for (auto &i: T) {
            auto s = e_closure(i);
            for (auto &j: s) {
                result.insert(j);
            }
        }
        return result;
    }

    std::vector<NFA *> NFA2DFA::Move(const std::set<int> &T, char c) {
        std::set<int> state_set;
        std::vector<NFA *> result;
        for (auto &n: T) {
            for (auto &l: (*nfa_[n])->links_) {
                if (l.first == c) {
                    state_set.insert(l.second);
                }
            }
        }
        result.reserve(state_set.size());
        for (auto &i: state_set)
            result.push_back(*nfa_[i]);
        return result;
    }

    int NFA2DFA::Get_DStates(const std::set<int> &states) {
        for (auto &i: D_states_) {
            if ((*i)->NFAStates_ == states)
                return (*i)->state_;
        }
        return -1;
    }

    void NFA2DFA::ShowDFA() {
        printf("----------DFA---------\n");
        for (auto &i: D_tran_) {
            printf("state %d: [", (*i)->state_);
            for (auto &j: (*D_states_[(*i)->state_])->NFAStates_)
                printf("%d,", j);
            printf("], next state: {");
            for (auto &j: (*i)->links_) {
                printf("---%c--->%d, ", j.first, j.second);
            }
            printf("}\n");
        }
    }

    vector<shared_ptr<DFA *>> &NFA2DFA::getDFA() {
        return D_tran_;
    }
}
