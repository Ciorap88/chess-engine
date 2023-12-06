#pragma once

#ifndef MOVEUTILS_H_
#define MOVEUTILS_H_

class MoveUtils {
public:
    static int getMove(int from, int to, bool color, int fromPiece, int capturedPiece, int prom, bool castle, bool ep);

    static bool isCastle(int move);
    static bool isEP(int move);
    static bool isCapture(int move);
    static bool isPromotion(int move);

    static int getFromSq(int move);
    static int getToSq(int move);
    static int getColor(int move);
    static int getPiece(int move);
    static int getCapturedPiece(int move);
    static int getPromotionPiece(int move);
};

#endif