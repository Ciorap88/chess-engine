#pragma once

#ifndef BOARD_H_
#define BOARD_H_

#include <bits/stdc++.h>

using namespace std;

typedef unsigned long long U64;

enum Piece {
    Empty = 0,
    Pawn = 1,
    Knight = 2,
    Bishop = 3,
    Rook = 4,
    Queen = 5,
    King = 6,

    White = 8,
    Black = 16
};

enum squares {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

struct Move {
    int from, to, capture;
    bool ep, castle;
    int prom;
};

class Board {
public:
    int squares[64], turn;

    int whiteKingSquare, blackKingSquare;
    U64 pawnsBB, knightsBB, bishopsBB, rooksBB, queensBB;
    U64 blackPiecesBB, whitePiecesBB;

    U64 hashKey;

    int ep;

    // bit 0 is white short, 1 is white long, 2 is black short and 3 is black long
    int castleRights;

    void initZobristHashFromCurrPos();

    void updatePieceInBB(int piece, int color, int sq);
    void movePieceInBB(int piece, int color, int from, int to);

    void loadFenPos(string pieces, char turn, string castles, string epTargetSq, int halfMoveClock, int fullMoveNumber);
    string getFenFromCurrPos();

    U64 attacksTo(int sq);
    bool isAttacked(int sq);
    bool isInCheck();

    vector<Move> GeneratePseudoLegalMoves();
    vector<Move> GenerateLegalMoves();

    void makeMove(Move m);
    void unmakeMove(Move m, int ep, int castleRights);
};

extern Board board;
extern U64 bits[64], filesBB[8], ranksBB[8], knightAttacksBB[64], kingAttacksBB[64];
extern U64 squaresNearWhiteKing[64], squaresNearBlackKing[64];
extern U64 lightSquaresBB, darkSquaresBB;
extern U64 bishopMasks[64], rookMasks[64];

U64 pawnAttacks(U64 pawns, int color);
U64 knightAttacks(U64 knights);

bool isInBoard(int sq, int dir);
int popcount(U64 bb);
int popcount(U64 bb);
bool putsKingInCheck(Move a);
int moveGenTest(int depth, bool show);
void Init();

string square(int x);
string moveToString(Move m);

#endif
