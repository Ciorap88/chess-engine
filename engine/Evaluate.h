#pragma once

#ifndef EVALUATE_H_
#define EVALUATE_H_

#include "Board.h"

#include <unordered_map>

extern int PIECE_VALUES[7];
extern const int MG_WEIGHT[7];

int gamePhase();
int evaluate();

int evaluate(
    bool usePawnHash, 

    int KING_SAFETY_TABLE[100], 
    int MG_KING_TABLE[64], int EG_KING_TABLE[64],
    int QUEEN_TABLE[64], int ROOK_TABLE[64], int BISHOP_TABLE[64], 
    int KNIGHT_TABLE[64], int MG_PAWN_TABLE[64], int EG_PAWN_TABLE[64], int PASSED_PAWN_TABLE[64],

    int KING_SHIELD[3],

    int& KNGIHT_MOBILITY, int& KNIGHT_PAWN_CONST, int& TRAPPED_KNIGHT_PENALTY,
    int& KNIGHT_DEF_BY_PAWN, int& BLOCKING_C_KNIGHT, int& KNIGHT_PAIR_PENALTY, 

    int& BISHOP_PAIR, int& TRAPPED_BISHOP_PENALTY, int& FIANCHETTO_BONUS, 
    int& BISHOP_MOBILITY, int& BLOCKED_BISHOP_PENALTY,

    int& ROOK_ON_QUEEN_FILE, int& ROOK_ON_OPEN_FILE, int& ROOK_PAWN_CONST,
    int& ROOK_ON_SEVENTH, int& ROOKS_DEF_EACH_OTHER, int& ROOK_MOBILITY,
    int& BLOCKED_ROOK_PENALTY,

    int& EARLY_QUEEN_DEVELOPMENT, int& QUEEN_MOBILITY,

    int& DOUBLED_PAWNS_PENALTY, int& WEAK_PAWN_PENALTY, int& C_PAWN_PENALTY,

    int& TEMPO_BONUS
);

#endif

