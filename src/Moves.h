#pragma once

#ifndef MOVES_H_
#define MOVES_H_

int getMove(char from, char to, bool color, char fromPiece, char capturedPiece, char prom, bool castle, bool ep);

bool isCastle(int move);
bool isEP(int move);
bool isCapture(int move);
bool isPromotion(int move);

char getFromSq(int move);
char getToSq(int move);
char getColor(int move);
char getPiece(int move);
char getCapturedPiece(int move);
char getPromotionPiece(int move);

#endif