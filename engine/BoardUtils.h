#pragma once

#ifndef BOARDUTILS_H_
#define BOARDUTILS_H_

#include "Board.h"

class BoardUtils {
public:
    static U64 bits[64], filesBB[8], ranksBB[8], knightAttacksBB[64], kingAttacksBB[64], whitePawnAttacksBB[64], blackPawnAttacksBB[64];
    static U64 squaresNearWhiteKing[64], squaresNearBlackKing[64];
    static U64 lightSquaresBB, darkSquaresBB;
    static U64 bishopMasks[64], rookMasks[64], castleMask[4];
    const static int castleStartSq[4], castleEndSq[4];


    static U64 eastOne(U64 bb);
    static U64 westOne(U64 bb);
    static U64 northOne(U64 bb);
    static U64 southOne(U64 bb);

    static U64 pawnAttacks(U64 pawns, int color);
    static U64 knightAttacks(U64 knights);

    static bool isInBoard(int sq, int dir);
    static void init();

    static int direction(int from, int to);
    static string square(int x);
    static string moveToString(int move);
};

#endif