#include <bits/stdc++.h>

#include "Evaluate.h"
#include "Board.h"

using namespace std;

vector<int> pieceValues = {0,100,300,310,500,900,0};

const int weight[7] = {0,0,1,1,2,4,0};

int Evaluate() {
    return 0;
}

const int knightMobilityConstant = 10;
const int knightPawnConstant = 2;
const int trappedKnightPenalty = 100;
const int knightDefendedByPawn = 15;
const int knightCloseToEnemyKing = 50;
const int blockingCKnight = 30;

int evalKnight(int sq, int color) {
    U64 opponentPawnsBB = (board.pawnsBB & (color == White ? board.blackPiecesBB : board.whitePiecesBB));
    U64 ourPawnsBB = (board.pawnsBB ^ opponentPawnsBB);
    U64 ourPiecesBB = (color == White ? board.whitePiecesBB : board.blackPiecesBB);

    U64 ourPawnAttacksBB = pawnAttacks(ourPawnsBB, color);
    U64 opponentPawnAttacksBB = pawnAttacks(opponentPawnsBB, (color ^ (White | Black)));

    int eval = pieceValues[Knight];

    // mobility bonus
    U64 mob = (knightAttacksBB[sq] ^ (knightAttacksBB[sq] & (ourPiecesBB | opponentPawnAttacksBB)));
    eval += knightMobilityConstant * (popcount(mob) - 4);

    // decreasing value as pawns disappear
    int numberOfPawns = popcount(board.pawnsBB);
    eval += knightPawnConstant * numberOfPawns;

    // penalty if trapped in the corner or it blocks the c2/c7 pawn from advancing
    if(color == White) {
        if(sq == 56 && (opponentPawnsBB & (bits[50] | bits[48])))
           eval -= trappedKnightPenalty; // a8
        if(sq == 48 && (opponentPawnsBB & (bits[40] | bits[42])) && (opponentPawnsBB & (bits[49] | bits[51])))
            eval -= trappedKnightPenalty; // a7

        if(sq == 63 && (opponentPawnsBB & (bits[53] | bits[55])))
           eval -= trappedKnightPenalty; // h8
        if(sq == 55 && (opponentPawnsBB & (bits[45] | bits[47])) && (opponentPawnsBB & (bits[52] | bits[54])))
            eval -= trappedKnightPenalty; // h7

        if(sq == 18 && (ourPawnsBB & bits[10]) && (ourPawnsBB & bits[27]) && !(ourPawnsBB & bits[28]))
            eval -= blockingCKnight; // c3 knight blocking c2 pawn
    }
    if(color == Black) {
        if(sq == 0 && (opponentPawnsBB & (bits[8] | bits[10])))
           eval -= trappedKnightPenalty; // a1
        if(sq == 8 && (opponentPawnsBB & (bits[16] | bits[18])) && (opponentPawnsBB & (bits[9] | bits[11])))
            eval -= trappedKnightPenalty; // a2

        if(sq == 7 && (opponentPawnsBB & (bits[13] | bits[15])))
           eval -= trappedKnightPenalty; // h1
        if(sq == 15 && (opponentPawnsBB & (bits[21] | bits[23])) && (opponentPawnsBB & (bits[12] | bits[14])))
            eval -= trappedKnightPenalty; // h2
        if(sq == 50 && (ourPawnsBB & bits[58]) && (ourPawnsBB & bits[43]) && !(ourPawnsBB & bits[44]))
            eval -= blockingCKnight; // c6 knight blocking c7 pawn
    }

    // bonus if defended by pawns
    if(ourPawnAttacksBB & bits[sq])
        eval += knightDefendedByPawn;

    // bonus if it is close to the king
    int enemyKingSquare = (color == White ? board.blackKingSquare : board.whiteKingSquare);
    if(dist(sq, enemyKingSquare) <= 4) eval += knightCloseToEnemyKing;

    return eval;
}

const int trappedBishopPenalty = 150;
const int fianchettoBonus = 30;
const int bishopMobilityConstant = 5;
const int attackingBishopBonus = 5;

int evalBishop(int sq, int color) {
    U64 ourPawnsBB = (board.whitePiecesBB & board.pawnsBB);
    U64 opponentPawnsBB = (board.blackPiecesBB & board.pawnsBB);
    if(color == Black) swap(ourPawnsBB, opponentPawnsBB);

    U64 ourPiecesBB = (color == White ? board.whitePiecesBB : board.blackPiecesBB);
    U64 opponentPiecesBB = (color == Black ? board.whitePiecesBB : board.blackPiecesBB);

    int opponentKingSquare = (color == White ? board.blackKingSquare : board.whiteKingSquare);


    int eval = pieceValues[Bishop];

    // trapped bishop penalty
    if(color == White) {
        if(sq == 55 && (opponentPawnsBB & bits[53]) && (opponentPawnsBB & bits[46]))
            eval -= trappedBishopPenalty; // h7
        if(sq == 48 && (opponentPawnsBB & bits[50]) && (opponentPawnsBB & bits[41]))
            eval -= trappedBishopPenalty; // a7
    }
    if(color == Black) {
        if(sq == 15 && (opponentPawnsBB & bits[13]) && (opponentPawnsBB & bits[22]))
            eval -= trappedBishopPenalty; // h2
        if(sq == 8 && (opponentPawnsBB & bits[10]) && (opponentPawnsBB & bits[17]))
            eval -= trappedBishopPenalty; // a2
    }

    // fianchetto bonus (bishop on long diagonal on the second rank)
    if(color == White && (sq == 9 || sq == 14)) eval += fianchettoBonus;
    if(color == Black && (sq ==  49|| sq == 54)) eval += fianchettoBonus;

    // mobility and squares attacked near the opponent king
    int mobility = 0;
    int attackedSquares = 0;
    for(int dir: piecesDirs[Bishop]) {
        if(!isInBoard(sq, dir)) continue;

        int curSq = sq+dir;
        while(true) {
            if(!(bits[curSq] & ourPiecesBB)) mobility++;
            if(opponentKingSquare == curSq || (kingAttacksBB[opponentKingSquare] & bits[curSq])) attackedSquares++;

            if(!isInBoard(curSq, dir) || (bits[curSq] & (opponentPiecesBB | ourPiecesBB))) break;
            curSq += dir;
        }
    }

    eval += bishopMobilityConstant * (mobility-7);
    eval += attackingBishopBonus * attackedSquares;

    return eval;
}

const int rookOnOpenFile = 20;
const int rookOnSeventh = 80;
const int rookPawnConstant = 2;
const int rooksDefendingEachOther = 5;
const int attackingRookBonus = 10;
const int rookMobilityConstant = 3;

int evalRook(int sq, int color) {
    U64 currFileBB = filesBB[sq%8];
    U64 currRankBB = ranksBB[sq/8];

    U64 ourPiecesBB = (color == White ? board.whitePiecesBB : board.blackPiecesBB);
    U64 opponentPiecesBB = (color == Black ? board.whitePiecesBB : board.blackPiecesBB);
    U64 ourPawnsBB = (board.whitePiecesBB & board.pawnsBB);
    U64 opponentPawnsBB = (board.blackPiecesBB & board.pawnsBB);
    if(color == Black) swap(ourPawnsBB, opponentPawnsBB);

    int opponentKingSquare = (color == White ? board.blackKingSquare : board.whiteKingSquare);

    // in this case seventh rank means the second rank in the opponent's half
    int seventhRank = (color == White ? 6 : 1);
    int eightRank = (color == White ? 7 : 0);

    int eval = pieceValues[Rook];

    // the rook becomes more valuable as there are less pawns on the board
    int numberOfPawns = popcount(board.pawnsBB);
    eval += rookPawnConstant * (16 - numberOfPawns);

    // bonus for a rook on an open or semi open file
    bool ourBlockingPawns = (currFileBB & ourPawnsBB);
    bool opponentBlockingPawns = (currFileBB & opponentPawnsBB);

    if(!ourBlockingPawns) {
        if(opponentBlockingPawns) eval += rookOnOpenFile/2; // semi open file
        else eval += rookOnOpenFile; // open file
    }

    // the rook on the seventh rank gets a huge bonus if there are pawns on that rank or if it restricts the king to the eighth rank
    if(sq/8 ==  seventhRank && (opponentKingSquare/8 == eightRank || (opponentPawnsBB & ranksBB[seventhRank])))
        eval += rookOnSeventh;

    // small bonus if the rook is defended by another rook
    if((board.rooksBB & ourPiecesBB & (currRankBB | currFileBB)) ^ bits[sq])
        eval += rooksDefendingEachOther;

    // mobility and squares attacked near the opponent king
    int mobility = 0;
    int attackedSquares = 0;
    for(int dir: piecesDirs[Rook]) {
        if(!isInBoard(sq, dir)) continue;

        int curSq = sq+dir;
        while(true) {
            if(!(bits[curSq] & ourPiecesBB)) mobility++;
            if(opponentKingSquare == curSq || (kingAttacksBB[opponentKingSquare] & bits[curSq])) attackedSquares++;

            if(!isInBoard(curSq, dir) || (bits[curSq] & (opponentPiecesBB | ourPiecesBB))) break;
            curSq += dir;
        }
    }

    eval += rookMobilityConstant * (mobility-7);
    eval += attackingRookBonus * attackedSquares;

    return eval;
}
