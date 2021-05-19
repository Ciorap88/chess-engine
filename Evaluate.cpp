#include <bits/stdc++.h>

#include "Evaluate.h"
#include "Board.h"

using namespace std;

vector<int> pieceValues = {0,100,300,310,500,900,0};

const int pawnTable[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0,
};

const int knightTable[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
};

const int bishopTable[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
};

const int rookTable[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      5, 10, 10, 10, 10, 10, 10,  5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      0,  0,  0,  5,  5,  0,  0,  0
};

const int queenTable[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

const int mgKingTable[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

const int egKingTable[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

const int weight[7] = {0,0,1,1,2,4,0};

// full tables for black pieces (need to flip squares for white)
int table[7][64];

int flip(int square) {
    int Rank = square/8;
    square -= 8*Rank;

    Rank = 7-Rank;
    square += 8*Rank;

    return square;
}

void initTables() {
    for(int piece = Pawn; piece < King; piece++)
        for(int i = 0; i < 64; i++)
            table[piece][i] = pieceValues[piece];


    for(int i = 0; i < 64; i++) {
        table[Pawn][i] += pawnTable[i];
        table[Knight][i] += knightTable[i];
        table[Bishop][i] += bishopTable[i];
        table[Rook][i] += rookTable[i];
        table[Queen][i] += queenTable[i];
    }
}

int Evaluate() {
    int evalWhite = 0, evalBlack = 0;
    int mgWeight = 0;

    for(int i = 0; i < 64; i++) {
        if(board.squares[i] == Empty) continue;

        int color = (board.squares[i] & (Black | White));
        int piece = (board.squares[i] ^ color);

        if(piece == King) continue;

        if(color == White) {
            evalWhite += table[piece][flip(i)];
        }
        if(color == Black) {
            evalBlack += table[piece][i];
        }

        mgWeight += weight[piece];
    }

    // middle game king value
    if(mgWeight > 10) {
        evalWhite += mgKingTable[flip(board.whiteKingSquare)];
        evalBlack += mgKingTable[board.blackKingSquare];

    // end game
    } else {
        evalWhite += egKingTable[flip(board.whiteKingSquare)];
        evalBlack += egKingTable[board.blackKingSquare];
    }

    int perspective = (board.turn == White ? 1 : -1);

    return perspective * (evalWhite - evalBlack);
}
