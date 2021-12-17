//
// Created by wyyadd on 12/15/21.
//

#include "DFA2mDFA.h"

#include <utility>

namespace Lexical {

    DFA2mDFA::DFA2mDFA(vector<shared_ptr<DFA *>> DFA, set<char> charSet) : DFA_(std::move(DFA)),
                                                                           charSet_(std::move(charSet)) {}

    void DFA2mDFA::GenerateGroup() {
        set<int> acceptSet;
        // init
        auto *ac = new Group;
        auto *no_ac = new Group;
        for (auto &i: DFA_) {
            if ((*i)->acceptable)
                ac->member_.insert((*i)->state_);
            else
                no_ac->member_.insert((*i)->state_);
        }
        stack<Group *> G_stack;
        G_stack.push(ac);
        G_stack.push(no_ac);
        // divide
        while (!G_stack.empty()) {
            auto g = G_stack.top();
            G_stack.pop();
            bool g_delete = false;
            for (auto &c: charSet_) {
                // true means cannot divide
                // false means can divide
                if (!DivideGroup(g, c, G_stack)) {
                    g_delete = true;
                    break;
                }
            }
            // mean not deleted
            if (!g_delete)
                Group_.emplace_back(std::make_unique<Group *>(g));
        }
    }

    bool DFA2mDFA::DivideGroup(Group *g, char c, stack<Group *> &s) {
        if (g->member_.size() == 1)
            return true;
        auto *A = new Group;
        for (auto i: g->member_) {
            auto dfa = *DFA_[i];
            for (auto j: dfa->links_) {
                if (j.first == c) {
                    if (!g->member_.contains(j.second)) {
                        A->member_.insert(dfa->state_);
                        break;
                    }
                }
            }
        }
        if (A->member_.empty()) {
            return true;
        } else {
            if (A->member_.size() == g->member_.size())
                return true;
            auto *B = new Group;
            for (auto i: g->member_) {
                if (!A->member_.contains(i))
                    B->member_.insert(i);
            }
            s.push(A);
            s.push(B);
            return false;
        }
    }

    void DFA2mDFA::Generate_mDFA() {
        int state = 0;
        for (auto i = 0; i < Group_.size(); ++i)
            mDFA_.emplace_back(std::make_unique<mDFA *>(new mDFA));
        for (auto &i: Group_) {
            mDFA *mDfa = *mDFA_[state];
            mDfa->state_ = state++;
            // set entry and exit
            for (auto &dfa: (*i)->member_) {
                if ((*DFA_[dfa])->state_ == 0) {
                    entry = mDfa;
                    break;
                }
            }
            for (auto &dfa: (*i)->member_) {
                if ((*DFA_[dfa])->acceptable) {
                    mDfa->acceptable = true;
                    break;
                }
            }
            // init mDFA
            auto dfa = *DFA_[*(*i)->member_.begin()];
            for (auto &j: dfa->links_)
                mDfa->links_.emplace_back(std::make_pair(j.first, *mDFA_[getGroupState(j.second)]));
        }
        // unreachable
        ReduceRedundancy(entry);
        // delete unreachable node
        for (int i = (int) mDFA_.size() - 1; i >= 0; --i) {
            if ((*mDFA_[i])->dead) {
                mDFA_.erase(std::next(mDFA_.begin(), i));
            }
        }
        // set exit set
        for (auto &m_dfa: mDFA_) {
            auto m = *m_dfa;
            if ((m->acceptable))
                exit.insert(m);
        }
        // loop itself
        CacheInfiniteLoop();
    }

    int DFA2mDFA::getGroupState(int s) {
        for (auto i = 0; i < Group_.size(); ++i) {
            if ((*Group_[i])->member_.contains(s))
                return i;
        }
        throw std::runtime_error("error");
    }

    void DFA2mDFA::ReduceRedundancy(mDFA *start) {
        if (!start->dead) {
            return;
        }
        start->dead = false;
        for (auto i: start->links_) {
            ReduceRedundancy(i.second);
        }
    }

    void DFA2mDFA::CacheInfiniteLoop() {
        // loop itself node
        for (auto &i: mDFA_) {
            bool flag = true;
            for (auto j: (*i)->links_) {
                if (j.second != *i) {
                    flag = false;
                    break;
                }
            }
            if (flag && !exit.contains(*i))
                (*i)->infiniteLoop = true;
        }
    }

    void DFA2mDFA::Show_mDFA() {
        printf("----------mDFA---------\n");
        printf("start state:%d, end state:", entry->state_);
        for (auto &e: exit)
            printf("%d,", e->state_);
        printf("\n");
        for (auto &i: mDFA_) {
            printf("state %d:{ ", (*i)->state_);
            for (auto j: (*i)->links_) {
                printf("---%c--->%d, ", j.first, j.second->state_);
            }
            printf("}\n");
        }
    }

    bool DFA2mDFA::Check(const std::string &s) {
        return walkThroughDFA(s, 0, entry);
    }

    bool DFA2mDFA::walkThroughDFA(const std::string &s, int po, mDFA *m_dfa) {
        if (exit.contains(m_dfa) && po == s.size())
            return true;
        if (m_dfa->infiniteLoop)
            return false;
        for (auto &i: m_dfa->links_) {
            if (i.first == s[po]) {
                return walkThroughDFA(s, po + 1, i.second);
            }
        }
        return false;
    }
}