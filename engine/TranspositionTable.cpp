#include <iostream>
#include <vector>
#include <unordered_map>
#include <cassert>

#include "Board.h"
#include "TranspositionTable.h"
#include "Search.h"
#include "MagicBitboardUtils.h"
#include "MoveUtils.h"
#include "BoardUtils.h"
#include "Enums.h"

using namespace std;

typedef unsigned long long U64;

U64 TranspositionTable::pieceZobristNumbers[7][2][64];
U64 TranspositionTable::castleZobristNumbers[16];
U64 TranspositionTable::epZobristNumbers[8];
U64 TranspositionTable::blackTurnZobristNumber;

const int TranspositionTable::VAL_UNKNOWN = -1e9;
const int TranspositionTable::HASH_F_EXACT = 0;
const int TranspositionTable::HASH_F_ALPHA = 1;
const int TranspositionTable::HASH_F_BETA = 2;
const int TranspositionTable::HASH_F_UNKNOWN = -1;

struct TranspositionTable::hashElement {
    U64 key;
    short depth;
    int flags;
    int value;
    int best;
};

TranspositionTable::TranspositionTable(): SIZE(1 << 22), hashTable(new hashElement[SIZE]) {
    clear();
}

TranspositionTable::~TranspositionTable() {
    delete[] hashTable;
}

void TranspositionTable::generateZobristHashNumbers() {
    for(int pc = 0; pc < 7; pc++) {
        for(int c = 0; c < 2; c++) {
            for(int sq = 0; sq < 64; sq++) {
                TranspositionTable::pieceZobristNumbers[pc][c][sq] = MagicBitboardUtils::randomULL();
            }
        }
    }
    for(int castle = 0; castle < 16; castle++) {
        TranspositionTable::castleZobristNumbers[castle] = MagicBitboardUtils::randomULL();
    }
    for(int col = 0; col < 8; col++) {
        TranspositionTable::epZobristNumbers[col] = MagicBitboardUtils::randomULL();
    }
    TranspositionTable::blackTurnZobristNumber = MagicBitboardUtils::randomULL();
}

// get the best move from the tt
int TranspositionTable::retrieveBestMove() {
    int index = (board->hashKey & (SIZE-1));
    assert(index >= 0 && index < SIZE);

    hashElement *h = &hashTable[index];

    if(h->key != board->hashKey) return MoveUtils::NO_MOVE;

    return h->best;
}

// check if the stored hash element corresponds to the current position and if it was searched at a good enough depth
int TranspositionTable::probeHash(short depth, int alpha, int beta) {
    int index = (board->hashKey & (SIZE-1));
    assert(index >= 0 && index < SIZE);

    hashElement *h = &hashTable[index];

    if(h->key == board->hashKey) {
        if(h->depth >= depth) {
            // return max(min(h->value, beta), alpha);
            if((h->flags == HASH_F_EXACT || h->flags == HASH_F_ALPHA) && h->value <= alpha) return alpha;
            if((h->flags == HASH_F_EXACT || h->flags == HASH_F_BETA) && h->value >= beta) return beta;
            if(h->flags == HASH_F_EXACT && h->value > alpha && h->value < beta) return h->value;
        }
    }
    return VAL_UNKNOWN;
}

// replace hashed element if replacement conditions are met
void TranspositionTable::recordHash(short depth, int val, int hashF, int best) {
    if(Search::timeOver) return;

    int index = (board->hashKey & (SIZE-1));
    assert(index >= 0 && index < SIZE);

    hashElement *h = &hashTable[index];

    bool skip = false;
    // if(h->depth > depth) skip = true;
    if((h->key == board->hashKey) && (h->depth > depth)) skip = true; 
    if(hashF != HASH_F_EXACT && h->flags == HASH_F_EXACT) skip = true; 
    if(hashF == HASH_F_EXACT && h->flags != HASH_F_EXACT) skip = false;
    if(h->flags == HASH_F_UNKNOWN) skip = false;
    if(skip) return;

    h->key = board->hashKey;
    h->best = best;
    h->value = val;
    h->flags = hashF;
    h->depth = depth;
}

void TranspositionTable::clear() {
    hashElement newElement = {0, -2, HASH_F_UNKNOWN, 0, MoveUtils::NO_MOVE};
    for(int i = 0; i < SIZE; i++) {
        hashTable[i] = newElement;
    }
}

TranspositionTable *transpositionTable = nullptr;