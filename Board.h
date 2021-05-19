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

struct Move {
    int from, to, capture;
    bool ep, castle;
    int prom;
};

class Board {
public:
    int squares[64], turn;
    int whiteKingSquare, blackKingSquare;

    U64 pawns, knights, bishops, rooks, queens, kings;
    U64 blackPieces, whitePieces;
    U64 zobristHash;

    int ep;

    // bit 0 is white short, 1 is white long, 2 is black short and 3 is black long
    int castleRights;

    // TODO: add pieces array to lookup all the pieces faster

    void initZobristHashFromCurrPos();

    void LoadFenPos(string fen);
    string getFenFromCurrPos();

    bool isInCheck();

    vector<Move> GeneratePseudoLegalMoves();
    vector<Move> GenerateLegalMoves();

    void makeMove(Move m);
    void unmakeMove(Move m, int ep, int castleRights);
};

extern Board board;

bool putsKingInCheck(Move a);
int moveGenTest(int depth, bool show);
void Init();

string square(int x);
string moveToString(Move m);

#endif
