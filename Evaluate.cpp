#include <bits/stdc++.h>

#include "Evaluate.h"
#include "Board.h"

using namespace std;

vector<int> pieceValues = {0,100,300,310,500,900,0};

const int weight[7] = {0,0,1,1,2,4,0};

// flipped squares for piece suqare tables for black
const int flipped[64] = {
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18 ,19, 20, 21, 22, 23,
    8, 9, 10, 11 , 12, 13, 14, 15,
    0, 1, 2, 3, 4, 5, 6, 7
};

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

    // traps and blockages
    if(color == White) {
        if(sq == a8 && (opponentPawnsBB & (bits[a7] | bits[c7])))
           eval -= trappedKnightPenalty;
        if(sq == a7 && (opponentPawnsBB & (bits[a6] | bits[c6])) && (opponentPawnsBB & (bits[b7] | bits[d7])))
            eval -= trappedKnightPenalty;

        if(sq == h8 && (opponentPawnsBB & (bits[h7] | bits[f7])))
           eval -= trappedKnightPenalty;
        if(sq == h7 && (opponentPawnsBB & (bits[f6] | bits[h6])) && (opponentPawnsBB & (bits[e7] | bits[g7])))
            eval -= trappedKnightPenalty;

        if(sq == c3 && (ourPawnsBB & bits[c2]) && (ourPawnsBB & bits[d4]) && !(ourPawnsBB & bits[e4]))
            eval -= blockingCKnight;
    }
    if(color == Black) {
        if(sq == a1 && (opponentPawnsBB & (bits[a2] | bits[c2])))
           eval -= trappedKnightPenalty;
        if(sq == a2 && (opponentPawnsBB & (bits[a3] | bits[c3])) && (opponentPawnsBB & (bits[b2] | bits[d2])))
            eval -= trappedKnightPenalty;

        if(sq == h1 && (opponentPawnsBB & (bits[h2] | bits[f2])))
           eval -= trappedKnightPenalty;
        if(sq == h2 && (opponentPawnsBB & (bits[f3] | bits[h3])) && (opponentPawnsBB & (bits[e2] | bits[g2])))
            eval -= trappedKnightPenalty;

        if(sq == c6 && (ourPawnsBB & bits[c7]) && (ourPawnsBB & bits[d5]) && !(ourPawnsBB & bits[e5]))
            eval -= blockingCKnight;
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
const int blockedBishopPenalty = 30;

int evalBishop(int sq, int color) {
    U64 ourPawnsBB = (board.whitePiecesBB & board.pawnsBB);
    U64 opponentPawnsBB = (board.blackPiecesBB & board.pawnsBB);
    if(color == Black) swap(ourPawnsBB, opponentPawnsBB);

    U64 ourPiecesBB = (color == White ? board.whitePiecesBB : board.blackPiecesBB);
    U64 opponentPiecesBB = (color == Black ? board.whitePiecesBB : board.blackPiecesBB);

    int opponentKingSquare = (color == White ? board.blackKingSquare : board.whiteKingSquare);


    int eval = pieceValues[Bishop];

    // traps and blockages
    if(color == White) {
        if(sq == a7 && (opponentPawnsBB & bits[b6]) && (opponentPawnsBB & bits[c7]))
            eval -= trappedBishopPenalty;
        if(sq == h7 && (opponentPawnsBB & bits[g6]) && (opponentPawnsBB & bits[f7]))
            eval -= trappedBishopPenalty;

        if(sq == c1 && (ourPawnsBB & bits[d2]) & (ourPiecesBB & bits[d3]))
            eval -= blockedBishopPenalty;
        if(sq == f1 && (ourPawnsBB & bits[e2]) & (ourPiecesBB & bits[e3]))
            eval -= blockedBishopPenalty;
    }
    if(color == Black) {
        if(sq == a2 && (opponentPawnsBB & bits[b3]) && (opponentPawnsBB & bits[c2]))
            eval -= trappedBishopPenalty;
        if(sq == h2 && (opponentPawnsBB & bits[g3]) && (opponentPawnsBB & bits[f2]))
            eval -= trappedBishopPenalty;

        if(sq == c8 && (ourPawnsBB & bits[d7]) & (ourPiecesBB & bits[d6]))
            eval -= blockedBishopPenalty;
        if(sq == f8 && (ourPawnsBB & bits[e7]) & (ourPiecesBB & bits[e6]))
            eval -= blockedBishopPenalty;
    }

    // fianchetto bonus (bishop on long diagonal on the second rank)
    if(color == White && (sq == b2 || sq == g2)) eval += fianchettoBonus;
    if(color == Black && (sq ==  b7 || sq == g7)) eval += fianchettoBonus;

    // mobility and squares attacked near the opponent king
    int mobility = 0;
    int attackedSquares = 0;
    for(int dir: piecesDirs[Bishop]) {
        if(!isInBoard(sq, dir)) continue;

        int curSq = sq+dir;
        while(true) {
            if(!(bits[curSq] & ourPiecesBB)) mobility++;
            if(opponentKingSquare == curSq || (kingAttacksBB[opponentKingSquare] & bits[curSq]))
                attackedSquares++;

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
const int blockedRookPenalty = 30;

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

    // blocked by uncastled king
    if(color == White) {
        if((board.whiteKingSquare == f1 || board.whiteKingSquare == g1) && (sq == g1 || sq == h1))
            eval -= blockedRookPenalty;
        if((board.whiteKingSquare == c1 || board.whiteKingSquare == b1) && (sq == a1 || sq == b1))
            eval -= blockedRookPenalty;
    }
    if(color == Black) {
        if((board.whiteKingSquare == f8 || board.whiteKingSquare == g8) && (sq == g8 || sq == h8))
            eval -= blockedRookPenalty;
        if((board.whiteKingSquare == c8 || board.whiteKingSquare == b8) && (sq == a8 || sq == b8))
            eval -= blockedRookPenalty;
    }

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
            if(opponentKingSquare == curSq || (kingAttacksBB[opponentKingSquare] & bits[curSq]))
                attackedSquares++;

            if(!isInBoard(curSq, dir) || (bits[curSq] & (opponentPiecesBB | ourPiecesBB))) break;
            curSq += dir;
        }
    }

    eval += rookMobilityConstant * (mobility-7);
    eval += attackingRookBonus * attackedSquares;

    return eval;
}

const int attackingQueenBonus = 15;
const int earlyQueenDevelopment = 5;
const int queenMobilityConstant = 3;

int evalQueen(int sq, int color) {
    U64 ourPiecesBB = (color == White ? board.whitePiecesBB : board.blackPiecesBB);
    U64 opponentPiecesBB = (color == Black ? board.whitePiecesBB : board.blackPiecesBB);
    U64 ourBishopsBB = (board.bishopsBB & ourPiecesBB);
    U64 ourKnightsBB = (board.knightsBB & ourPiecesBB);

    int opponentKingSquare = (color == Black ? board.whiteKingSquare : board.blackKingSquare);

    int eval = pieceValues[Queen];

    // penalty for early development
    if(color == White && sq/8 > 1) {
        if(ourKnightsBB & bits[b1]) eval -= earlyQueenDevelopment;
        if(ourBishopsBB & bits[c1]) eval -= earlyQueenDevelopment;
        if(ourBishopsBB & bits[f1]) eval -= earlyQueenDevelopment;
        if(ourKnightsBB & bits[g1]) eval -= earlyQueenDevelopment;
    }
    if(color == Black && sq/8 < 6) {
        if(ourKnightsBB & bits[b8]) eval -= earlyQueenDevelopment;
        if(ourBishopsBB & bits[c8]) eval -= earlyQueenDevelopment;
        if(ourBishopsBB & bits[f8]) eval -= earlyQueenDevelopment;
        if(ourKnightsBB & bits[g8]) eval -= earlyQueenDevelopment;
    }

    // mobility and squares attacked near the opponent king
    int mobility = 0;
    int attackedSquares = 0;
    for(int dir: piecesDirs[Queen]) {
        if(!isInBoard(sq, dir)) continue;

        int curSq = sq+dir;
        while(true) {
            if(!(bits[curSq] & ourPiecesBB)) mobility++;
            if(opponentKingSquare == curSq || (kingAttacksBB[opponentKingSquare] & bits[curSq]))
                attackedSquares++;

            if(!isInBoard(curSq, dir) || (bits[curSq] & (opponentPiecesBB | ourPiecesBB))) break;
            curSq += dir;
        }
    }

    eval += queenMobilityConstant * (mobility-14);
    eval += attackingQueenBonus * attackedSquares;

    return eval;
}

const int shield1 = 10;
const int shield2 = 5;
const int noShield = 5;

int whiteKingShield() {
    U64 ourPawnsBB = (board.whitePiecesBB & board.pawnsBB);
    int sq = board.whiteKingSquare;

    int eval = 0;
    // queen side
    if(sq%8 < 3) {
        if(ourPawnsBB & bits[a2]) eval += shield1;
        else if(ourPawnsBB & bits[a3]) eval += shield2;
        else eval -= noShield;

        if(ourPawnsBB & bits[b2]) eval += shield1;
        else if(ourPawnsBB & bits[b3]) eval += shield2;
        else eval -= noShield;

        if(ourPawnsBB & bits[c2]) eval += shield1;
        else if(ourPawnsBB & bits[c3]) eval += shield2;
        else eval -= noShield;
    }

    // king side
    else if(sq%8 > 4) {
        if(ourPawnsBB & bits[f2]) eval += shield1;
        else if(ourPawnsBB & bits[f3]) eval += shield2;
        else eval -= noShield;

        if(ourPawnsBB & bits[g2]) eval += shield1;
        else if(ourPawnsBB & bits[g3]) eval += shield2;
        else eval -= noShield;

        if(ourPawnsBB & bits[h2]) eval += shield1;
        else if(ourPawnsBB & bits[h3]) eval += shield2;
        else eval -= noShield;
    }
    return eval;
}

int blackKingShield() {
    U64 ourPawnsBB = (board.blackPiecesBB & board.pawnsBB);
    int sq = board.blackKingSquare;

    int eval = 0;
    // queen side
    if(sq%8 < 3) {
        if(ourPawnsBB & bits[a7]) eval += shield1;
        else if(ourPawnsBB & bits[a6]) eval += shield2;
        else eval -= noShield;

        if(ourPawnsBB & bits[b7]) eval += shield1;
        else if(ourPawnsBB & bits[b6]) eval += shield2;
        else eval -= noShield;

        if(ourPawnsBB & bits[c7]) eval += shield1;
        else if(ourPawnsBB & bits[c6]) eval += shield2;
        else eval -= noShield;
    }

    // king side
    else if(sq%8 > 4) {
        if(ourPawnsBB & bits[f7]) eval += shield1;
        else if(ourPawnsBB & bits[f6]) eval += shield2;
        else eval -= noShield;

        if(ourPawnsBB & bits[g7]) eval += shield1;
        else if(ourPawnsBB & bits[g6]) eval += shield2;
        else eval -= noShield;

        if(ourPawnsBB & bits[h7]) eval += shield1;
        else if(ourPawnsBB & bits[h6]) eval += shield2;
        else eval -= noShield;
    }
    return eval;
}

const int doubledPawnsPenalty = 20;
const int weakPawnPenalty = 15;
const int passedPawnTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20,
    40, 40, 40, 40, 40, 40, 40, 40,
    60, 60, 60, 60, 60, 60, 60, 60,
    80, 80, 80, 80, 80, 80, 80, 80,
    100, 100, 100, 100, 100, 100, 100, 100,
    0, 0, 0, 0, 0, 0, 0, 0,
};

int evalPawn(int sq, int color) {
    U64 ourPiecesBB = (color == White ? board.whitePiecesBB : board.blackPiecesBB);

    U64 opponentPawnsBB = (board.pawnsBB & (color == White ? board.blackPiecesBB : board.whitePiecesBB));
    U64 ourPawnsBB = (board.pawnsBB & (color == White ? board.whitePiecesBB : board.blackPiecesBB));

    U64 opponentPawnAttacksBB = pawnAttacks(opponentPawnsBB, (color == White ? Black : White));
    U64 ourPawnAttacksBB = pawnAttacks(ourPawnsBB, color);

    bool weak = true, passed = true, opposed = false;

    int eval = pieceValues[Pawn];
    int dir = (color == White ? 8 : -8);

    // check squares in front of the pawn to see if it is passed or opposed/doubled
    int curSq = sq+dir;
    while(curSq < 64 && curSq >= 0) {
        if(board.pawnsBB & bits[curSq]) {
            passed = false;
            if(ourPiecesBB & bits[curSq]) eval -= doubledPawnsPenalty;
            else opposed = true;
        }
        if(opponentPawnAttacksBB & bits[curSq]) passed = false;

        curSq += dir;
    }

    // check squares behind the pawn to see if it can be protected by other pawns
    curSq = sq;
    while(curSq < 64 && curSq >= 0) {
        if(ourPawnAttacksBB & curSq) {
            weak = false;
            break;
        }
        curSq -= dir;
    }

    // bonus for passed pawns, bigger bonus for protected passers
    // the bonus is also bigger if the pawn is more advanced
    if(passed) {
        int bonus = passedPawnTable[(color == White ? sq : flipped[sq])];
        if(ourPawnAttacksBB & bits[sq]) bonus = (bonus*4)/3;

        eval += bonus;
    }

    // penalty for weak (backward or isolated) pawns and bigger penalty if they are on a semi open file
    if(weak) {
        int penalty = weakPawnPenalty;
        if(!opposed) penalty = (penalty*4)/3;

        eval -= penalty;
    }

    return eval;
}
