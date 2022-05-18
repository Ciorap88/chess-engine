#pragma once

#ifndef SEARCH_H_
#define SEARCH_H_

#include <bits/stdc++.h>

using namespace std;

pair<int, int> search();
void clearHistory();

extern const int NO_MOVE, MATE_THRESHOLD, MATE_EVAL;

extern short maxDepth;
extern long long startTime, stopTime;
extern const int bestMove;
extern bool infiniteTime, timeOver;

#endif
