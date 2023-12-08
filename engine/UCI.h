#pragma once

#ifndef UCI_H_
#define UCI_H_

#include <string>
#include <mutex>
#include <condition_variable>

class UCI {
private:
    static std::condition_variable cv;
    static std::mutex mtx;
    static bool startFlag, quitFlag;

    static std::string goCommand;
    static std::string engineName;

public:
    static void UCICommunication();
    static void inputUCI();
    static void inputIsReady();
    static void inputUCINewGame();
    static void inputPosition(std::string input);
    static void inputGo();

    static void showSearchInfo(short depth, int nodes, int startTime, int score);
    static long long moveGenTest(short depth, bool show);
    static void printBoard(bool chars);
    static void printEval();
};

#endif