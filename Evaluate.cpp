#include <bits/stdc++.h>

#include "Evaluate.h"
#include "Board.h"
#include "MagicBitboards.h"

using namespace std;

const int kingSafetyTable[100] = {
    0, 0, 1, 2, 3, 5, 7, 9, 12, 15,
    18, 22, 26, 30, 35, 39, 44, 50, 56, 62,
    68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
    140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
    260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
    377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
    494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};

const int mgKingTable[64] = {
    40, 50, 30, 10, 10, 30, 50, 40,
    30, 40, 20, 0, 0, 20, 40, 30,
    10, 20, 0, -20, -20, 0, 20, 10,
    0, 10, -10, -30, -30, -10, 10, 0,
    -10, 0, -20, -40, -40, -20, 0, -10,
    -20, -10, -30, -50, -50, -30, -10, -20,
    -30, -20, -40, -60, -60, -40, -20, -30,
    -40, -30, -50, -70, -70, -50, -30, -40
};

const int egKingTable[64] = {
    -72, -48, -36, -24, -24, -36, -48, -72,
    -48, -24, -12, 0, 0, -12, -24, -48,
    -36, -12, 0, 12, 12, 0, -12, -36,
    -24, 0, 12, 24, 24, 12, 0, -24,
    -24, 0, 12, 24, 24, 12, 0, -24,
    -36, -12, 0, 12, 12, 0, -12, -36,
    -48, -24, -12, 0, 0, -12, -24, -48,
    -72, -48, -36, -24, -24, -36, -48, -72
};

const int queenTable[64] = {
    -5, -5, -5, -5, -5, -5, -5, -5,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 1, 2, 2, 1, 0, 0,
    0, 0, 2, 3, 3, 2, 0, 0,
    0, 0, 2, 3, 3, 2, 0, 0,
    0, 0, 1, 2, 2, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

const int rookTable[64] = {
    0, 0, 0, 2, 2, 0, 0, 0
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    20, 20, 20, 20, 20, 20, 20, 20,
    5, 5, 5, 5, 5, 5, 5, 5,
};

const int bishopTable[64] = {
    -4, -4, -12, -4, -4, -12, -4, -4
    -4, 2, 1, 1, 1, 1, 2, -4,
    -4, 0, 2, 4, 4, 2, 0, -4,
    -4, 0, 4, 6, 6, 4, 0, -4,
    -4, 0, 4, 6, 6, 4, 0, -4,
    -4, 1, 2, 4, 4, 2, 1, -4,
    -4, 0, 0, 0, 0, 0, 0, -4,
    -4, -4, -4, -4, -4, -4, -4, -4,
};

const int knightTable[64] = {
    -8, -12, -8, -8, -8, -8, -12, -8
    -8, 0, 0, 0, 0, 0, 0, -8,
    -8, 0, 4, 4, 4, 4, 0, -8,
    -8, 0, 4, 8, 8, 4, 0, -8,
    -8, 0, 4, 8, 8, 4, 0, -8,
    -8, 0, 4, 4, 4, 4, 0, -8,
    -8, 0, 1, 2, 2, 1, 0, -8,
    -8, -8, -8, -8, -8, -8, -8, -8,
};

const int pawnTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    -6, -4, 1, -24, -24, 1, -4, -6,
    -4, -4, 1, 5, 5, 1, -4, -4,
    -6, -4, 5, 10, 10, 5, -4, -6,
    -6, -4, 2, 8, 8, 2, -4, -6,
    -6, -4, 1, 2, 2, 1, -4, -6,
    -6, -4, 1, 1, 1, 1, -4, -6,
    0, 0, 0, 0, 0, 0, 0, 0
};

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

// flipped squares for piece square tables for black
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


const int mgWeight[7] = {0, 0, 1, 1, 2, 4, 0};
int pieceValues[7] = {0, 100, 300, 310, 500, 900, 0};
int pieceAttackWeight[6] = {0, 0, 2, 2, 3, 5};

const int knightMobilityConstant = 3;
const int knightPawnConstant = 3;
const int trappedKnightPenalty = 100;
const int knightDefendedByPawn = 15;
const int blockingCKnight = 30;
const int knightPairPenalty = 20;

const int bishopPairBonus = 50;
const int trappedBishopPenalty = 150;
const int fianchettoBonus = 20;
const int bishopMobilityConstant = 3;
const int blockedBishopPenalty = 30;

const int rookOnQueenFile = 10;
const int rookOnOpenFile = 20;
const int rookOnSeventh = 60;
const int rookPawnConstant = 3;
const int rooksDefendingEachOther = 5;
const int rookMobilityConstant = 3;
const int blockedRookPenalty = 30;

const int earlyQueenDevelopment = 10;
const int queenMobilityConstant = 3;

const int shield1 = 10;
const int shield2 = 5;
const int noShield = 5;

const int doubledPawnsPenalty = 20;
const int weakPawnPenalty = 15;

const int tempoBonus = 10;


int gamePhase, whiteAttackersCnt, blackAttackersCnt, whiteAttackWeight, blackAttackWeight;
int pawnCntWhite, pawnCntBlack, pieceMaterialWhite, pieceMaterialBlack;

int evalPawn(int sq, int color);
int evalKnight(int sq, int color);
int evalBishop(int sq, int color);
int evalRook(int sq, int color);
int evalQueen(int sq, int color);
int whiteKingShield(), blackKingShield();


int Evaluate() {

    // reset everything
    gamePhase = 0;
    whiteAttackersCnt = blackAttackersCnt = 0;
    whiteAttackWeight = blackAttackWeight = 0;
    pawnCntWhite = pawnCntBlack = 0;
    pieceMaterialWhite = pieceMaterialBlack = 0;

    // evaluate pieces independently
    int res = 0;
    for(int sq = 0; sq < 64; sq++) {
        if(board.squares[sq] == Empty) continue;

        int color = (board.squares[sq] & (Black | White));
        int c = (color == White ? 1 : -1);
        int oldRes = res;
        if(board.pawnsBB & bits[sq]) res += evalPawn(sq, color) * c;
        if(board.knightsBB & bits[sq]) res += evalKnight(sq, color) * c;
        if(board.bishopsBB & bits[sq]) res += evalBishop(sq, color) * c;
        if(board.rooksBB & bits[sq]) res += evalRook(sq, color) * c;
        if(board.queensBB & bits[sq]) res += evalQueen(sq, color) * c;
    }

    // evaluate kings based on the current game phase (king centralization becomes more important than safety as pieces disappear from the board)
    int mgWeight = min(gamePhase, 24);
    int egWeight = 24-mgWeight;

    int mgKingScore = whiteKingShield() - blackKingShield();
    int egKingScore = 0;

    // evaluate king safety in the middlegame

    // if only 1 or 2 attackers, we consider the king safe
    if(whiteAttackersCnt <= 2) whiteAttackWeight = 0;
    if(blackAttackersCnt <= 2) blackAttackWeight = 0;

    mgKingScore += kingSafetyTable[whiteAttackWeight] - kingSafetyTable[blackAttackWeight];
    mgKingScore += mgKingTable[board.whiteKingSquare] - mgKingTable[flipped[board.blackKingSquare]];

    egKingScore += egKingTable[board.whiteKingSquare] - egKingTable[flipped[board.blackKingSquare]];

    res += (mgWeight * mgKingScore + egWeight * egKingScore) / 24;

    // tempo bonus
    if(board.turn == White) res += tempoBonus;
    else res -= tempoBonus;

    // add scores for bishop and knight pairs
    if(popcount(board.whitePiecesBB & board.bishopsBB) >= 2) res += bishopPairBonus;
    if(popcount(board.blackPiecesBB & board.bishopsBB) >= 2) res -= bishopPairBonus;

    if(popcount(board.whitePiecesBB & board.knightsBB) >= 2) res -= knightPairPenalty;
    if(popcount(board.blackPiecesBB & board.knightsBB) >= 2) res += knightPairPenalty;

    // low material corrections (adjusting the score for well known draws)

    int strongerSide = White, weakerSide = Black;
    int strongerPawns = pawnCntWhite, weakerPawns = pawnCntBlack;
    int strongerPieces = pieceMaterialWhite, weakerPieces = pieceMaterialBlack;
    if(res < 0) {
        swap(strongerSide, weakerSide);
        swap(strongerPieces, weakerPieces);
        swap(strongerPawns, weakerPawns);
    }

    if(strongerPawns == 0) {
        // weaker side cannot be checkmated
        if(strongerPieces < 400) return 0;
        if(weakerPawns == 0 && weakerPieces == 2*pieceValues[Knight]) return 0;

        // rook vs minor piece
        if(strongerPieces == pieceValues[Rook] && (weakerPieces == pieceValues[Knight] || weakerPieces == pieceValues[Bishop]) )
            res /= 2;

        // rook and minor vs rook
        if((strongerPieces == pieceValues[Rook] + pieceValues[Bishop] || strongerPieces == pieceValues[Rook] + pieceValues[Knight])
           && weakerPieces == pieceValues[Rook])
            res /= 2;
    }
    // return result from the perspective of the side to move
    if(board.turn == Black) res *= -1;

    return res;
}

int evalKnight(int sq, int color) {
    U64 opponentPawnsBB = (board.pawnsBB & (color == White ? board.blackPiecesBB : board.whitePiecesBB));
    U64 ourPawnsBB = (board.pawnsBB ^ opponentPawnsBB);
    U64 ourPiecesBB = (color == White ? board.whitePiecesBB : board.blackPiecesBB);

    U64 ourPawnAttacksBB = pawnAttacks(ourPawnsBB, color);
    U64 opponentPawnAttacksBB = pawnAttacks(opponentPawnsBB, (color ^ (White | Black)));

    gamePhase += mgWeight[Knight];
    if(color == White) pieceMaterialWhite += pieceValues[Knight];
    else pieceMaterialBlack += pieceValues[Knight];

    // initial piece value and square value
    int eval = pieceValues[Knight] + knightTable[(color == White ? sq : flipped[sq])];


    // mobility bonus
    U64 mob = (knightAttacksBB[sq] ^ (knightAttacksBB[sq] & (ourPiecesBB | opponentPawnAttacksBB)));
    eval += knightMobilityConstant * (popcount(mob) - 4);

    // decreasing value as pawns disappear
    int numberOfPawns = popcount(board.pawnsBB);
    eval += knightPawnConstant * (numberOfPawns - 8);

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

    // attacks
    U64 sqNearKing = (color == White ? squaresNearBlackKing[board.blackKingSquare] : squaresNearWhiteKing[board.whiteKingSquare]);

    int attackedSquares = popcount(knightAttacksBB[sq] & sqNearKing);
    if(attackedSquares) {
        if(color == White) {
            whiteAttackersCnt++;
            whiteAttackWeight += pieceAttackWeight[Knight] * popcount(attackedSquares);
        } else {
            blackAttackersCnt++;
            blackAttackWeight += pieceAttackWeight[Knight] * popcount(attackedSquares);
        }
    }

    return eval;
}

int evalBishop(int sq, int color) {
    U64 ourPawnsBB = (board.whitePiecesBB & board.pawnsBB);
    U64 opponentPawnsBB = (board.blackPiecesBB & board.pawnsBB);
    if(color == Black) swap(ourPawnsBB, opponentPawnsBB);

    U64 ourPiecesBB = (color == White ? board.whitePiecesBB : board.blackPiecesBB);
    U64 opponentPiecesBB = (color == Black ? board.whitePiecesBB : board.blackPiecesBB);

    int opponentKingSquare = (color == White ? board.blackKingSquare : board.whiteKingSquare);

    gamePhase += mgWeight[Bishop];
    if(color == White) pieceMaterialWhite += pieceValues[Bishop];
    else pieceMaterialBlack += pieceValues[Bishop];

    // initial piece value and square value
    int eval = pieceValues[Bishop] + bishopTable[(color == White ? sq : flipped[sq])];

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
    if(color == White && sq == g2 && (ourPawnsBB & bits[g3]) && (ourPawnsBB & bits[f2])) eval += fianchettoBonus;
    if(color == White && sq == b2 && (ourPawnsBB & bits[b3]) && (ourPawnsBB & bits[c2])) eval += fianchettoBonus;
    if(color == Black && sq == g7 && (ourPawnsBB & bits[g6]) && (ourPawnsBB & bits[f7])) eval += fianchettoBonus;
    if(color == Black && sq == b7 && (ourPawnsBB & bits[b6]) && (ourPawnsBB & bits[c7])) eval += fianchettoBonus;

    // mobility and attacks
    U64 sqNearKing = (color == White ? squaresNearBlackKing[board.blackKingSquare] : squaresNearWhiteKing[board.whiteKingSquare]);
    U64 attacks = magicBishopAttacks((board.whitePiecesBB | board.blackPiecesBB), sq);

    int mobility = popcount(attacks & ~ourPiecesBB);
    int attackedSquares = popcount(attacks & sqNearKing);

    eval += bishopMobilityConstant * (mobility-7);
    if(attackedSquares) {
        if(color == White) {
            whiteAttackersCnt++;
            whiteAttackWeight += pieceAttackWeight[Bishop] * attackedSquares;
        } else {
            blackAttackersCnt++;
            blackAttackWeight += pieceAttackWeight[Bishop] * attackedSquares;
        }
    }

    return eval;
}

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
    int eighthRank = (color == White ? 7 : 0);

    gamePhase += mgWeight[Rook];
    if(color == White) pieceMaterialWhite += pieceValues[Rook];
    else pieceMaterialBlack += pieceValues[Rook];

    // initial piece value and square value
    int eval = pieceValues[Rook] + rookTable[(color == White ? sq : flipped[sq])];

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
    eval += rookPawnConstant * (8 - numberOfPawns);

    // bonus for a rook on an open or semi open file
    bool ourBlockingPawns = (currFileBB & ourPawnsBB);
    bool opponentBlockingPawns = (currFileBB & opponentPawnsBB);

    if(!ourBlockingPawns) {
        if(opponentBlockingPawns) eval += rookOnOpenFile/2; // semi open file
        else eval += rookOnOpenFile; // open file
    }

    // the rook on the seventh rank gets a huge bonus if there are pawns on that rank or if it restricts the king to the eighth rank
    if(sq/8 ==  seventhRank && (opponentKingSquare/8 == eighthRank || (opponentPawnsBB & ranksBB[seventhRank])))
        eval += rookOnSeventh;

    // small bonus if the rook is defended by another rook
    if((board.rooksBB & ourPiecesBB & (currRankBB | currFileBB)) ^ bits[sq])
        eval += rooksDefendingEachOther;

    // bonus for a rook that is on the same file as the enemy queen
    if(currFileBB & opponentPiecesBB & board.queensBB) eval += rookOnQueenFile;

    // mobility and attacks
    U64 sqNearKing = (color == White ? squaresNearBlackKing[board.blackKingSquare] : squaresNearWhiteKing[board.whiteKingSquare]);
    U64 attacks = magicRookAttacks((board.whitePiecesBB | board.blackPiecesBB), sq);

    int mobility = popcount(attacks & ~ourPiecesBB);
    int attackedSquares = popcount(attacks & sqNearKing);

    eval += rookMobilityConstant * (mobility-7);
    if(attackedSquares) {
        if(color == White) {
            whiteAttackersCnt++;
            whiteAttackWeight += pieceAttackWeight[Rook] * attackedSquares;
        } else {
            blackAttackersCnt++;
            blackAttackWeight += pieceAttackWeight[Rook] * attackedSquares;
        }
    }

    return eval;
}

int evalQueen(int sq, int color) {
    U64 ourPiecesBB = (color == White ? board.whitePiecesBB : board.blackPiecesBB);
    U64 opponentPiecesBB = (color == Black ? board.whitePiecesBB : board.blackPiecesBB);
    U64 ourBishopsBB = (board.bishopsBB & ourPiecesBB);
    U64 ourKnightsBB = (board.knightsBB & ourPiecesBB);

    int opponentKingSquare = (color == Black ? board.whiteKingSquare : board.blackKingSquare);

    gamePhase += mgWeight[Knight];
    if(color == White) pieceMaterialWhite += pieceValues[Queen];
    else pieceMaterialBlack += pieceValues[Queen];

    // initial piece value and square value
    int eval = pieceValues[Queen] + queenTable[(color == White ? sq : flipped[sq])];

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

    // mobility and attacks
    U64 sqNearKing = (color == White ? squaresNearBlackKing[board.blackKingSquare] : squaresNearWhiteKing[board.whiteKingSquare]);
    U64 attacks = (magicBishopAttacks((board.whitePiecesBB | board.blackPiecesBB), sq) | magicRookAttacks((board.whitePiecesBB | board.blackPiecesBB), sq));

    int mobility = popcount(attacks & ~ourPiecesBB);
    int attackedSquares = popcount(attacks & sqNearKing);

    eval += queenMobilityConstant * (mobility-14);
    if(attackedSquares) {
        if(color == White) {
            whiteAttackersCnt++;
            whiteAttackWeight += pieceAttackWeight[Queen] * attackedSquares;
        } else {
            blackAttackersCnt++;
            blackAttackWeight += pieceAttackWeight[Queen] * attackedSquares;
        }
    }
    return eval;
}

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

int evalPawn(int sq, int color) {
    U64 ourPiecesBB = (color == White ? board.whitePiecesBB : board.blackPiecesBB);

    U64 opponentPawnsBB = (board.pawnsBB & (color == White ? board.blackPiecesBB : board.whitePiecesBB));
    U64 ourPawnsBB = (board.pawnsBB & (color == White ? board.whitePiecesBB : board.blackPiecesBB));

    U64 opponentPawnAttacksBB = pawnAttacks(opponentPawnsBB, (color == White ? Black : White));
    U64 ourPawnAttacksBB = pawnAttacks(ourPawnsBB, color);

    bool weak = true, passed = true, opposed = false;

    if(color == White) pawnCntWhite ++;
    else pawnCntBlack ++;

    // initial pawn value + square value
    int eval = pieceValues[Pawn] + pawnTable[(color == White ? sq : flipped[sq])];
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
