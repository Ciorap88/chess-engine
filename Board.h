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
    bool castleWK, castleWQ, castleBK, castleBQ;

    // TODO: add pieces array to lookup all the pieces faster


    void LoadFenPos(string fen);
    string getFenFromCurrPos();

    bool isInCheck();

    vector<Move> GeneratePseudoLegalMoves();
    vector<Move> GenerateLegalMoves();

    void makeMove(Move m);
    void unmakeMove(Move m, int ep, bool castlingRights[4]);
};

extern Board board;

void generateZobristHashNumbers();

bool putsKingInCheck(Move a);
int moveGenTest(int depth);
void Init();

string square(int x);
string moveToString(Move m);

#endif
