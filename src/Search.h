#pragma once

#ifndef SEARCH_H_
#define SEARCH_H_

#include <bits/stdc++.h>

using namespace std;

pair<int, int> search();
void clearHistory();

extern const int NO_MOVE, MATE_THRESHOLD, MATE_EVAL;

extern short maxDepth;
extern int startTime, stopTime, bestMove;
extern bool infiniteTime, timeOver;

#endif
