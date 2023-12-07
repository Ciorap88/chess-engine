#pragma once

#ifndef TRANSPOSITIONTABLE_H_
#define TRANSPOSITIONTABLE_H_

class TranspositionTable {
private:
    const int SIZE;
    struct hashElement;
    hashElement *hashTable;
public:
    TranspositionTable();

    static U64 pieceZobristNumbers[7][2][64];
    static U64 castleZobristNumbers[16];
    static U64 epZobristNumbers[8];
    static U64 blackTurnZobristNumber;

    static const int VAL_UNKNOWN;
    static const int HASH_F_ALPHA, HASH_F_BETA, HASH_F_EXACT;

    int retrieveBestMove();
    int probeHash(short depth, int alpha, int beta);
    void recordHash(short depth, int val, int hashF, int best);
    void clear();

    static void generateZobristHashNumbers();
};

extern TranspositionTable transpositionTable;

int retrievePawnEval();
void recordPawnEval(int eval);

#endif
