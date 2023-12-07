// ---magic bitboards---
// they are a hashing technique for sliding piece attacks
// there are too many different positions so we can't store a unique attack for every single one
// so we compute some "magic" numbers that map positions with the same attack bitboards to the same key

#include <iostream>
#include <unordered_map>

using namespace std;

#include "Board.h"
#include "MagicBitboardUtils.h"
#include "BoardUtils.h"

// number of bits we need to shift when computing magic attacks
const int MagicBitboardUtils::ROOK_BITS[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

const int MagicBitboardUtils::BISHOP_BITS[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

// precalculated values for the magic numbers
C64 MagicBitboardUtils::BISHOP_MAGICS[64] = {577322812503966336, 2270049134510085, 38289964190400650, 158769899958174800, 565217696222208, 581529844573028484, 4036387455024235520,
581034909873737735, 1152926211925100032, 7219360397122405076, 18594945514614800, 4677221482548, 2379030914224099330, 75437566601986049, 2017614866464133392,
37154838677037057, 361415798805365248, 55204288533234706, 288793602392342658, 18577365878833152, 295003440814227600, 18084775848838144, 584343159787553905,
149182288558164096, 2490543513319768324, 1227530799180683298, 2534383697937920, 38289393498325024, 4521260549685264, 4613953211559199244, 36310560044222516,
300235935187082, 342854131000549544, 579277770624862656, 1158010053581602944, 369470803805184, 4629719109709465602, 1130332850229248, 4504708821224452,
94858312693064192, 658106793251053568, 4620768268477671554, 2380539501220018176, 17730044430336, 82199631030715400, 2252933761075712, 2451086107324974080,
36354261126227012, 378867522042068992, 2450030868203438096, 6917541673770156546, 1689967659188224, 1127033828802568, 4902520193605181440, 189437126369148944,
2328363309472161793, 74768267218944, 2891311257527783936, 1163617970535338272, 4613937844619182592, 90423983978136576, 1139163907769376, 638954298933377,
306262375445569664};

C64 MagicBitboardUtils::ROOK_MAGICS[64] = {72075324745056513, 18031991769276424, 648553543602538512, 4827894603405330436, 144117387368072224, 144117387233591696, 2377936887437328532,
3494804722568675456, 1688988448522752, 216243426004312064, 281754151682064, 2814784664895556, 18295890951276546, 281483701125376, 148900271274131524,
2306405960794767490, 35734136307724, 36312471564320848, 580964902097657859, 630513843974439168, 38280872784560640, 36592296770143232, 39582452248648,
2253998845403220, 180144269636411392, 306280513088260352, 1153506741147664640, 36288179474432, 378311169088161024, 5911288043798594, 1297599651226584065,
563508299968577, 18014949347426336, 4503737070534672, 2310347158873710592, 35227355332624, 655274295680828418, 36591781365682180, 288529589410482177,
1441152981377482884, 281758453071905, 35185714806784, 148623186420834304, 288335998004428809, 144397796925046789, 4612249282184085508, 580973156630790160,
108372333881851905, 2738224307573362816, 18084835975235648, 155660684047155456, 162422057219527168, 5630083658220672, 22799507507118336, 576462955890607104,
4403022283264, 2341947122781262081, 590534574161805350, 1152992972998967313, 2533292238703137, 311311393515635714, 576742261706981889, 144117666281429012,
281483588804737};

const int MagicBitboardUtils::BitTable[64] = {
    63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
    51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
    26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
    58, 20, 37, 17, 36, 8
};

U64 MagicBitboardUtils::mBishopAttacks[64][512], MagicBitboardUtils::mRookAttacks[64][4096];

// index of least significant set bit
int MagicBitboardUtils::bitscanForward(U64 bb) {
    return __builtin_ctzll(bb);
}

// combine 4 random 16bit numbers
U64 MagicBitboardUtils::randomULL() {
    U64 u1 = (U64)(rand()) & 0xFFFF;
    U64 u2 = (U64)(rand()) & 0xFFFF;
    U64 u3 = (U64)(rand()) & 0xFFFF;
    U64 u4 = (U64)(rand()) & 0xFFFF;
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

// & multiple random numbers so we get fewer bits
U64 MagicBitboardUtils::randomU64FewBits() {
    return (randomULL() & randomULL() & randomULL());
}

// number of set bits
int MagicBitboardUtils::popcount(U64 bb) {
    return __builtin_popcountll(bb);
}

// pop the ms1b fron a number
int MagicBitboardUtils::popFirstBit(U64 *bb) {
    U64 b = *bb ^ (*bb - 1);
    unsigned int fold = (unsigned) ((b & 0xffffffff) ^ (b >> 32));
    *bb &= (*bb - 1);
    return BitTable[(fold * 0x783a9b23) >> 26];
}

// function that transforms the index in the attack arrays into a bitboard
// it is actually the inverse of the function that we access the hashed attacks with
U64 MagicBitboardUtils::indexToU64(int index, int bits, U64 m) {
    U64 res = 0;
    for(int i = 0; i < bits; i++) {
        int j = popFirstBit(&m);
        if(index & (1ULL << i)) res |= (1ULL << j);
  }
  return res;
}

// manually calculated rook attacks
// we go in every direction until we find another piece
U64 MagicBitboardUtils::rookAttacks(int sq, U64 blockers) {
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

// manually calculated bishop attacks
U64 MagicBitboardUtils::bishopAttacks(int sq, U64 blockers) {
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

// populate the mBishopAttacks and mRookAttacks arrays with the correct attack bitboards 
// according to the current magic numbers
void MagicBitboardUtils::populateSlidingAttacks(int sq, int m, bool bishop) {
    U64 magic = (bishop ? BISHOP_MAGICS[sq] : ROOK_MAGICS[sq]);
    U64 mask = (bishop ? BoardUtils::bishopMasks[sq] : BoardUtils::rookMasks[sq]);
    int n = popcount(mask);
    U64 blockers[4096], a[4096], used[4096];

    for(int i = 0; i < (1 << n); i++) {
        blockers[i] = indexToU64(i, n, mask);
        a[i] = (bishop ? bishopAttacks(sq, blockers[i]) : rookAttacks(sq, blockers[i]));
    }

    for(int i = 0; i < 4096; i++) used[i] = 0;
    for(int i = 0; i < (1 << n); i++) {
        U64 j = ((blockers[i] * magic) >> (64 - m));
        if(used[j] == 0) used[j] = a[i];
    }
    for(int i = 0; i < (1 << n); i++) {
        if(bishop) mBishopAttacks[sq][i] = used[i];
        else mRookAttacks[sq][i] = used[i];
    }
}

// function that finds good magic numbers, using trial and error
// it is too slow so I don't use it every time the engine starts
// I used it once and copied the magic numbers into the arrays
U64 MagicBitboardUtils::findMagic(int sq, int m, int bishop) {
    U64 blockers[4096], a[4096], used[4096];

    U64 mask = (bishop ? BoardUtils::bishopMasks[sq] : BoardUtils::rookMasks[sq]);
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

// the actual functions that return the attack bitboards
// we first & the occupancy bb with the correct mask, so we only get the blockers in the attack directions
// after that, we multiply the result with the corresponding magic number and then we right shift it
U64 MagicBitboardUtils::magicBishopAttacks(U64 occ, int sq) {
    occ &= BoardUtils::bishopMasks[sq];
    occ *= BISHOP_MAGICS[sq];
    occ >>= 64-BISHOP_BITS[sq];
    return mBishopAttacks[sq][occ];
}

U64 MagicBitboardUtils::magicRookAttacks(U64 occ, int sq) {
    occ &= BoardUtils::rookMasks[sq];
    occ *= ROOK_MAGICS[sq];
    occ >>= 64-ROOK_BITS[sq];
    return mRookAttacks[sq][occ];
}

// the function we will call when initializing the engine
void MagicBitboardUtils::initMagics() {
    for(int i = 0; i < 64; i++) {
        populateSlidingAttacks(i, BISHOP_BITS[i], 1);
        populateSlidingAttacks(i, ROOK_BITS[i], 0); 
    }
}

// function for generating magic numbers
void MagicBitboardUtils::generateMagicNumbers() {
    cout << "BISHOP_MAGICS[64] = {";
    for(int i = 0; i <64; i++) {
        if(i%8 == 7) cout << '\n';
        cout << findMagic(i, BISHOP_BITS[i], 1);
        if(i < 63) cout << ", ";
    }
    cout << "};\n";
    cout << "ROOK_MAGICS[64] = {";
    for(int i = 0; i <64; i++) {
        if(i%8 == 7) cout << '\n';
        cout << findMagic(i, ROOK_BITS[i], 0);
        if(i < 63) cout << ", ";
    }
    cout << "};\n";
}

