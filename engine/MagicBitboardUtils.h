#pragma once

#ifndef MAGICBITBOARDS_H_
#define MAGICBITBOARDS_H_

typedef unsigned long long U64;
typedef const U64 C64;

class MagicBitboardUtils {
private:
    static const int ROOK_BITS[64], BISHOP_BITS[64];
    static const int BitTable[64];
    static C64 BISHOP_MAGICS[64], ROOK_MAGICS[64];
    static U64 mBishopAttacks[64][512], mRookAttacks[64][4096];

    static U64 randomU64FewBits();
    static int popFirstBit(U64 *bb);
    static U64 indexToU64(int index, int bits, U64 m);
    static U64 rookAttacks(int sq, U64 blockers);
    static U64 bishopAttacks(int sq, U64 blockers);
    static void populateSlidingAttacks(int sq, int m, bool bishop);
    static U64 findMagic(int sq, int m, int bishop);
    static void generateMagicNumbers();
public:
    static void initMagics();
    static U64 magicBishopAttacks(U64 occ, int sq);
    static U64 magicRookAttacks(U64 occ, int sq);
    static int popcount(U64 bb);
    static U64 randomULL();
    static int bitscanForward(U64 bb);
};


#endif

