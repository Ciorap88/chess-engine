#pragma once

#ifndef SEARCH_H_
#define SEARCH_H_

#include <bits/stdc++.h>

#include "Board.h"
#include "Evaluate.h"

pair<int, int> Search();

extern bool timeOver;
extern int bestMove;
extern const int noMove;
extern const int MATE_THRESHOLD;
extern const int mateEval;

extern short maxDepth;
extern int startTime, stopTime;
extern bool infiniteTime;

#endif
