#pragma once

#ifndef EVALUATE_H_
#define EVALUATE_H_

#include <bits/stdc++.h>

#include "Board.h"

extern const int PIECE_VALUES[7];
extern const int MG_WEIGHT[7];

int gamePhase();
int evaluate();

#endif

