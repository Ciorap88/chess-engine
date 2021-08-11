#pragma once

#ifndef TRANSPOSITIONTABLE_H_
#define TRANSPOSITIONTABLE_H_

Move retrieveBestMove();
int ProbeHash(int depth, int alpha, int beta, Move * best);
void RecordHash(int depth, int val, int hashF, Move best);
void showPV(Move firstMove);
void generateZobristHashNumbers();
U64 getZobristHashFromCurrPos();

extern const int valUnknown;
extern const int hashFAlpha, hashFBeta, hashFExact;

#endif
