#pragma once

#ifndef SEARCH_H_
#define SEARCH_H_

#include <bits/stdc++.h>

#include "Board.h"
#include "Evaluate.h"

pair<Move, int> Search();

extern bool timeOver;
extern Move bestMove;
extern const Move noMove;
extern const int MATE_THRESHOLD;
extern const int mateEval;
#endif
