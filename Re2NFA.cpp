#include "Re2NFA.h"
#include <iostream>
#include <stack>
#include <cassert>

namespace Lexical {
    Re2NFA::Re2NFA(string regex) {
        this->infix_regex_ = PreprocessInfixRegex(regex);
        this->suffix_regex_ = "";
        in_stack_priority_ = {
                {'#', 0},
                {'(', 1},
                {'|', 3},
                {'&', 5},
                {'*', 7},
                {')', 8}
        };
        in_coming_priority_ = {
                {'#', 0},
                {'(', 8},
                {'|', 2},
                {'&', 4},
                {'*', 6},
                {')', 1}
        };
    }

    string Re2NFA::PreprocessInfixRegex(string &regex) {
        unsigned int length = regex.size();
        unsigned int i = 0;
        char c1, c2;
        while (i < length - 1) {
            c1 = regex[i];
            if ((c1 >= 'a' && c1 <= 'z') || c1 == ')' || c1 == '*') {
                c2 = regex[i + 1];
                if ((c2 >= 'a' && c2 <= 'z') || c2 == '(') {
                    regex.insert(i + 1, "&");
                    length++;
                }
            }
            ++i;
        }
        return regex + "#";
    }

    void Re2NFA::Infix2Suffix() {
        std::stack<char> stack;
        stack.push('#');
        int i = 0;
        while (infix_regex_[i] != '#' || stack.top() != '#') {
            if (infix_regex_[i] >= 'a' && infix_regex_[i] <= 'z') {
                charSet_.insert(infix_regex_[i]);
                suffix_regex_ += infix_regex_[i];
                ++i;
                continue;
            }
            if (in_coming_priority_[infix_regex_[i]] > in_stack_priority_[stack.top()]) {
                stack.push(infix_regex_[i]);
                ++i;
                continue;
            }
            if (in_coming_priority_[infix_regex_[i]] < in_stack_priority_[stack.top()]) {
                suffix_regex_ += stack.top();
                stack.pop();
                continue;
            }
            if (in_coming_priority_[infix_regex_[i]] == in_stack_priority_[stack.top()]) {
                stack.pop();
                ++i;
            }
        }
        std::cout << "regex infix: " << infix_regex_ << std::endl;
        std::cout << "regex suffix: " << suffix_regex_ << std::endl;
    }

    void Re2NFA::LinkNFA(std::stack<NFASet> &stack) {
        auto B = stack.top();
        stack.pop();
        auto &A = stack.top();
        for (auto i: B.NFAs_head) {
            A.NFAs_tail->links_.emplace_back(i.first, i.second->state_);
            if (i.first == '~')
                A.NFAs_tail->epsilon_.emplace_back(i.second->state_);
        }
        A.NFAs_tail = B.NFAs_tail;
    }

    void Re2NFA::GenerateNFA() {
        int state = 0;
        std::stack<NFASet> stack;
        NFA *head = new NFA(state++);
        nfa_.emplace_back(std::make_unique<NFA *>(head));
        stack.push(NFASet(head, '0'));

        for (char i: suffix_regex_) {
            // 若为字母， 创建一个新的NFAnet，压入stack
            if (i >= 'a' && i <= 'z') {
                NFA *n = new NFA(state++);
                nfa_.emplace_back(std::make_unique<NFA *>(n));
                stack.push(NFASet(n, i));
                continue;
            }
            // 若为|，则将stack的top两个元素并行合并
            if (i == '|') {
                assert(stack.size() > 1);
                auto B = stack.top();
                stack.pop();
                auto A = stack.top();
                stack.pop();
                // 把B的head合并到A
                for (auto &j: B.NFAs_head)
                    A.NFAs_head.emplace_back(j);
                // n作为A,B共同的尾部
                NFA *n = new NFA(state++);
                nfa_.emplace_back(std::make_unique<NFA *>(n));
                B.NFAs_tail->links_.emplace_back(std::make_pair('~', n->state_));
                B.NFAs_tail->epsilon_.emplace_back(n->state_);
                A.NFAs_tail->links_.emplace_back(std::make_pair('~', n->state_));
                A.NFAs_tail->epsilon_.emplace_back(n->state_);
                A.NFAs_tail = n;
                B.NFAs_tail = n;
                stack.push(A);
                continue;
            }
            // 若为*，创建一个新的NFA，该NFASet尾部通过epsilon指向NFA， 同时该NFASet尾部指向头部，同时将头部置为epsilon
            if (i == '*') {
                assert(stack.size() > 1);
                // 创建一个新的NFA
                NFA *n = new NFA(state++);
                nfa_.emplace_back(std::make_unique<NFA *>(n));
                auto A = stack.top();
                stack.pop();
                // NFASet尾部通过epsilon指向NFA
                A.NFAs_tail->epsilon_.emplace_back(n->state_);
                A.NFAs_tail->links_.emplace_back(std::make_pair('~', n->state_));
                // NFASet尾部指向头部
                for (auto &j: A.NFAs_head) {
                    A.NFAs_tail->links_.emplace_back(j.first, j.second->state_);
                    if (j.first == '~')
                        A.NFAs_tail->epsilon_.emplace_back(j.second->state_);
                }
                A.NFAs_head.clear();
                // 同时将头部置为epsilon
                A.NFAs_head.emplace_back(std::make_pair('~', A.NFAs_tail));
                A.NFAs_tail = n;
                stack.push(A);
                continue;
            }
            // 若为&，则将stack的top两个元素串行合并
            if (i == '&') {
                assert(stack.size() > 1);
                LinkNFA(stack);
                continue;
            }
        }
        LinkNFA(stack);
    }

    void Re2NFA::ShowNFA() {
        printf("-----NFA-----\n(~ = epsilon)\n");
        for (auto &i: nfa_) {
            printf("state %d, next state: {", (*i)->state_);
            for (auto &j: (*i)->links_) {
                printf("--%c-->%d, ", j.first, j.second);
            }
            printf("}, epsilon: {");
            for (auto &k: (*i)->epsilon_) {
                printf("--~-->%d, ", k);
            }
            printf("}\n");
        }
    }

    vector<shared_ptr<NFA *>> &Re2NFA::getNfa() {
        return nfa_;
    }

    set<char> Re2NFA::getCharSet() {
        return charSet_;
    }

}// namespace Lexical
