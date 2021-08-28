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
};

#endif