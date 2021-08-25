#pragma once

#ifndef EVALUATE_H_
#define EVALUATE_H_

#include <bits/stdc++.h>

#include "Board.h"

extern int pieceValues[7];
extern int gamePhase;
extern const int endgameMaterial;

void initTables();
int Evaluate();

#endif

