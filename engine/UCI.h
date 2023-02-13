#pragma once

#ifndef UCI_H_
#define UCI_H_

#include <string>

using namespace std;

class UCI {
public:
    static string engineName;

    static void UCICommunication();
    static void inputUCI();
    static void inputIsReady();
    static void inputUCINewGame();
    static void inputPosition(string input);
    static void inputGo(string input);

    static void showSearchInfo(short depth, int nodes, int startTime, int score);
    static long long moveGenTest(short depth, bool show);
    static void printBoard(bool chars);
    static void printEval();
};

#endif