#pragma once

#ifndef TRANSPOSITIONTABLE_H_
#define TRANSPOSITIONTABLE_H_

Move retrieveBestMove();
int ProbeHash(int depth, int alpha, int beta);
void RecordHash(int depth, int val, int hashF, Move best);
void generateZobristHashNumbers();
U64 getZobristHashFromCurrPos();
void showPV(int depth);

extern const int valUnknown;
extern const int hashFAlpha, hashFBeta, hashFExact;

#endif
