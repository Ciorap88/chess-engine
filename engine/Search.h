#pragma once

#ifndef SEARCH_H_
#define SEARCH_H_

using namespace std;

pair<int, int> search();
int quiesce(int alpha, int beta);
void clearHistory();
void showPV(int depth);

extern const int NO_MOVE, MATE_THRESHOLD, MATE_EVAL;

extern short maxDepth;
extern long long startTime, stopTime;
extern const int bestMove;
extern bool infiniteTime, timeOver;

#endif
