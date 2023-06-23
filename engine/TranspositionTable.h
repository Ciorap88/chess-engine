#pragma once

#ifndef TRANSPOSITIONTABLE_H_
#define TRANSPOSITIONTABLE_H_

int retrieveBestMove();
int probeHash(short depth, int alpha, int beta);
void recordHash(short depth, int val, int hashF, int best);
int retrievePawnEval();
void recordPawnEval(int eval);
void generateZobristHashNumbers();
U64 getZobristHashFromCurrPos();
void clearTT();

extern const int VAL_UNKNOWN;
extern const int HASH_F_ALPHA, HASH_F_BETA, HASH_F_EXACT;

#endif
