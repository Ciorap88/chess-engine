#include <bits/stdc++.h>

using namespace std;

#include "Board.h"
#include "MagicBitboards.h"

typedef unsigned long long U64;
typedef const U64 C64;

const int rookBits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

const int bishopBits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

U64 rookMagics[64];
U64 bishopMagics[64];
U64 mBishopAttacks[64][512], mRookAttacks[64][4096];


U64 randomU64() {
    U64 u1 = (U64)(rand()) & 0xFFFF;
    U64 u2 = (U64)(rand()) & 0xFFFF;
    U64 u3 = (U64)(rand()) & 0xFFFF;
    U64 u4 = (U64)(rand()) & 0xFFFF;
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

U64 randomU64FewBits() {
    return (randomU64() & randomU64() & randomU64());
}

const int BitTable[64] = {
    63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
    51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
    26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
    58, 20, 37, 17, 36, 8
};

int popFirstBit(U64 *bb) {
    U64 b = *bb ^ (*bb - 1);
    unsigned int fold = (unsigned) ((b & 0xffffffff) ^ (b >> 32));
    *bb &= (*bb - 1);
    return BitTable[(fold * 0x783a9b23) >> 26];
}


U64 indexToU64(int index, int bits, U64 m) {
    U64 res = 0;
    for(int i = 0; i < bits; i++) {
        int j = popFirstBit(&m);
        if(index & (1ULL << i)) res |= (1ULL << j);
  }
  return res;
}

U64 rookAttacks(int sq, U64 blockers) {
    U64 res = 0;
    int Rank = sq/8, File = sq%8;
    for(int r = Rank+1; r <= 7; r++) {
        res |= (1ULL << (File + r*8));
        if(blockers & (1ULL << (File + r*8))) break;
    }
    for(int r = Rank-1; r >= 0; r--) {
        res |= (1ULL << (File + r*8));
        if(blockers & (1ULL << (File + r*8))) break;
    }
    for(int f = File+1; f <= 7; f++) {
        res |= (1ULL << (f + Rank*8));
        if(blockers & (1ULL << (f + Rank*8))) break;
    }
    for(int f = File-1; f >= 0; f--) {
        res |= (1ULL << (f + Rank*8));
        if(blockers & (1ULL << (f + Rank*8))) break;
    }
    return res;
}

U64 bishopAttacks(int sq, U64 blockers) {
    U64 res = 0;
    int Rank = sq/8, File = sq%8;
    for(int r = Rank+1, f = File+1; r <= 7 && f <= 7; r++, f++) {
        res |= (1ULL << (f + r*8));
        if(blockers & (1ULL << (f + r * 8))) break;
    }
    for(int r = Rank+1, f = File-1; r <= 7 && f >= 0; r++, f--) {
        res |= (1ULL << (f + r*8));
        if(blockers & (1ULL << (f + r * 8))) break;
    }
    for(int r = Rank-1, f = File+1; r >= 0 && f <= 7; r--, f++) {
        res |= (1ULL << (f + r*8));
        if(blockers & (1ULL << (f + r * 8))) break;
    }
    for(int r = Rank-1, f = File-1; r >= 0 && f >= 0; r--, f--) {
        res |= (1ULL << (f + r*8));
        if(blockers & (1ULL << (f + r * 8))) break;
    }
    return res;
}

U64 findMagic(int sq, int m, int bishop) {
    U64 blockers[4096], a[4096], used[4096];

    U64 mask = (bishop ? bishopMasks[sq] : rookMasks[sq]);
    int n = popcount(mask);

    for(int i = 0; i < (1 << n); i++) {
        blockers[i] = indexToU64(i, n, mask);
        a[i] = (bishop ? bishopAttacks(sq, blockers[i]) : rookAttacks(sq, blockers[i]));
    }

    bool fail;
    for(int k = 0; k < 100000000; k++) {
        U64 magic = randomU64FewBits();
        if(popcount((mask * magic) & 0xFF00000000000000ULL) < 6) continue;

        for(int i = 0; i < 4096; i++) used[i] = 0;
        int i;
        for(fail = false, i = 0; (!fail) && (i < (1 << n)); i++) {
            U64 j = ((blockers[i] * magic) >> (64 - m));
            if(used[j] == 0) used[j] = a[i];
            else if(used[j] != a[i]) fail = true;
        }
        if(!fail) {
            for(int i = 0; i < (1 << n); i++) {
                if(bishop) mBishopAttacks[sq][i] = used[i];
                else mRookAttacks[sq][i] = used[i];
            }

            return magic;
        }
    }

    cout << "Magic Number not found\n";
    return 0;
}

U64 magicBishopAttacks(U64 occ, int sq) {
    occ &= bishopMasks[sq];
    occ *= bishopMagics[sq];
    occ >>= 64-bishopBits[sq];
    return mBishopAttacks[sq][occ];
}

U64 magicRookAttacks(U64 occ, int sq) {
    occ &= rookMasks[sq];
    occ *= rookMagics[sq];
    occ >>= 64-rookBits[sq];

    return mRookAttacks[sq][occ];
}

void initMagics() {
    for(int i = 0; i < 64; i++) {
        bishopMagics[i] = findMagic(i, bishopBits[i], 1);
        rookMagics[i] = findMagic(i, rookBits[i], 0);
    }
}

