#include <iostream>
#include "Re2NFA.h"
#include "NFA2DFA.h"
#include "DFA2mDFA.h"

int main() {
    printf("正则表达式规则(regex rule): a-z, |, &, *, (, )\n");
    printf("(Enter a regular expression)请输入一个正则表达式规则:");
    std::string reg;
    std::cin >> reg;
//    Lexical::Re2NFA re2Nfa("(a|b)*(aa|bb)(a|b)*");
//    Lexical::Re2NFA re2Nfa("(a|b*)(aa|bb)(a|b)*a(b|c)d*");
//    Lexical::Re2NFA re2Nfa("ab*cd*");
    Lexical::Re2NFA re2Nfa(reg);
    printf("\n-----------------step 1 regex to NFA ---------------------\n");
    re2Nfa.Infix2Suffix();
    re2Nfa.GenerateNFA();
    re2Nfa.ShowNFA();
    printf("\n-----------------step 2 NFA to DFA ---------------------\n");
    Lexical::NFA2DFA nfa2Dfa(re2Nfa.getNfa(), re2Nfa.getCharSet());
    nfa2Dfa.GenerateDFA();
    nfa2Dfa.ShowDFA();
    printf("\n-----------------step 3 DFA to mDFA ---------------------\n");
    Lexical::DFA2mDFA dfa2MDfa(nfa2Dfa.getDFA(), re2Nfa.getCharSet());
    dfa2MDfa.GenerateGroup();
    dfa2MDfa.Generate_mDFA();
    dfa2MDfa.Show_mDFA();
    printf("\n-----------------step 4 Test ---------------------\n");
    std::cout << reg << std::endl;
    int choice = 0;
    while (true) {
        printf("(0-continue,-1-quit)0进行判断操作，-1退出, 请输入:");
        scanf("%d", &choice);
        if (choice == -1)
            break;
        printf("(simulation) 请输入判断字符串:");
        std::string s;
        std::cin >> s;
        if (dfa2MDfa.Check(s))
            printf("accept满足\n");
        else
            printf("reject不满足\n");
    }
    return 0;
}
