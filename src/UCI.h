#pragma once

#ifndef UCI_H_
#define UCI_H_

#include <bits/stdc++.h>

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
    static int moveGenTest(short depth, bool show);
    static void printBoard();
};

#endif