#pragma once

#ifndef EVALUATE_H_
#define EVALUATE_H_

#include <bits/stdc++.h>

#include "Board.h"

extern std::vector<int> pieceValues;
extern Board board;

void initTables();
int Evaluate();

#endif

