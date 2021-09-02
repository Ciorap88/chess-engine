#pragma once

#ifndef TRANSPOSITIONTABLE_H_
#define TRANSPOSITIONTABLE_H_

int retrieveBestMove();
int ProbeHash(short depth, int alpha, int beta);
void RecordHash(short depth, int val, char hashF, int best);
int retrievePawnEval(U64 pawns);
void recordPawnEval(U64 pawns, int eval);
void generateZobristHashNumbers();
U64 getZobristHashFromCurrPos();
void showPV(short depth);
void clearTT();

extern const int valUnknown;
extern const char hashFAlpha, hashFBeta, hashFExact;

#endif
