#pragma once

#ifndef MOVES_H_
#define MOVES_H_

const int CAPTURE_MASK = ((1 << 16) | (1 << 17) | (1 << 18));
const int PROM_MASK = ((1 << 19) | (1 << 20) | (1 << 21));
const int FROM_MASK = 63; // first 6 bits
const int TO_MASK = ((1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
const int PIECE_MASK = ((1 << 13) | (1 << 14) | (1 << 15));

int getMove(char from, char to, bool color, char fromPiece, char capturedPiece, char prom, bool castle, bool ep);

inline bool isCastle(int move) {return (move & bits[22]);}
inline bool isEP(int move) {return (move & bits[23]);}
inline bool isCapture(int move) {return (move & CAPTURE_MASK);}
inline bool isPromotion(int move) {return (move & PROM_MASK);}

inline char getFromSq(int move) {return (move & FROM_MASK);}
inline char getToSq(int move) {return ((move & TO_MASK) >> 6);}
inline char getColor(int move) {return ((move & bits[12]) >> 9);}
inline char getPiece(int move) {return ((move & PIECE_MASK) >> 13);}
inline char getCapturedPiece(int move) {return ((move & CAPTURE_MASK) >> 16);}
inline char getPromotionPiece(int move) {return ((move & PROM_MASK) >> 19);}

#endif