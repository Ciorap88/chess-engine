#pragma once

#ifndef MOVES_H_
#define MOVES_H_

int getMove(int from, int to, bool color, int fromPiece, int capturedPiece, int prom, bool castle, bool ep);

bool isCastle(int move);
bool isEP(int move);
bool isCapture(int move);
bool isPromotion(int move);

int getFromSq(int move);
int getToSq(int move);
int getColor(int move);
int getPiece(int move);
int getCapturedPiece(int move);
int getPromotionPiece(int move);

#endif