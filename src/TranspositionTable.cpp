#include <bits/stdc++.h>

#include "Board.h"
#include "TranspositionTable.h"
#include "Search.h"
#include "MagicBitboards.h"
#include "Moves.h"

using namespace std;

typedef unsigned long long U64;


// indices in zobristNumbers vector
const int zBlackTurnIndex = 12*64;
const int zCastleRightsIndex = 12*64+1;
const int zEpFileIndex = 12*64+17;

vector<U64> zobristNumbers;

void generateZobristHashNumbers() {
    for(int i = 0; i < 793; i++) {
        zobristNumbers.push_back(randomULL());
  }
}

int zPieceSquareIndex(char piece, char color, char square) {
    int idx = (2*(piece-1) + (int)(color == Black))*64 + square;
    return idx;
}

// xor zobrist numbers corresponding to all features of the current position
U64 getZobristHashFromCurrPos() {
    U64 key = 0;
    for(char i = 0; i < 64; i++) {
        char color = (board.squares[i] & (Black | White));
        char piece = (board.squares[i] ^ color);

        key ^= zobristNumbers[zPieceSquareIndex(piece, color, i)];
    }

    key ^= zobristNumbers[zCastleRightsIndex + board.castleRights];
    if(board.turn == Black) key ^= zobristNumbers[zBlackTurnIndex];

    char epFile = board.ep % 8;
    key ^= zobristNumbers[zEpFileIndex + epFile];

    return key;
}

// update the hash key after making a move
void Board::updateHashKey(int move) {
    if(move == noMove) { // null move
        this->hashKey ^= zobristNumbers[zEpFileIndex + (this->ep & 7)];
        this->hashKey ^= zobristNumbers[zBlackTurnIndex];
        return;
    }

    // get move info
    char from = getFromSq(move);
    char to = getToSq(move);

    char color = getColor(move);
    char piece = getPiece(move);
    char otherColor = (color ^ 8);
    char otherPiece = getCapturedPiece(move);

    bool isMoveEP = isEP(move);
    bool isMoveCapture = isCapture(move);
    bool isMoveCastle = isCastle(move);
    char promotionPiece = getPromotionPiece(move);

    char capturedPieceSquare = (isMoveEP ? (to + (color == White ? South : North)) : to);

    // update pieces
    this->hashKey ^= zobristNumbers[zPieceSquareIndex(piece, color, from)];
    if(isMoveCapture) this->hashKey ^= zobristNumbers[zPieceSquareIndex(otherPiece, otherColor, capturedPieceSquare)];

    if(!promotionPiece) this->hashKey ^= zobristNumbers[zPieceSquareIndex(piece, color, to)];
    else this->hashKey ^= zobristNumbers[zPieceSquareIndex(promotionPiece, color, to)];

    // castle stuff
    char newCastleRights = this->castleRights;
    if(piece == King) {
        char mask = (color == White ? 12 : 3);
        newCastleRights &= mask;
    }
    if((newCastleRights & bits[1]) && (from == a1 || to == a1))
        newCastleRights ^= bits[1];
    if((newCastleRights & bits[0]) && (from == h1 || to == h1))
        newCastleRights ^= bits[0];
    if((newCastleRights & bits[3]) && (from == a8 || to == a8))
        newCastleRights ^= bits[3];
    if((newCastleRights & bits[2]) && (from == h8 || to == h8))
        newCastleRights ^= bits[2];

    this->hashKey ^= zobristNumbers[zCastleRightsIndex + this->castleRights];
    this->hashKey ^= zobristNumbers[zCastleRightsIndex + newCastleRights];

    if(isMoveCastle) {
        char Rank = (to >> 3), File = (to & 7);
        char rookStartSquare = (Rank << 3) + (File == 6 ? 7 : 0);
        char rookEndSquare = (Rank << 3) + (File == 6 ? 5 : 3);

        this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, color, rookStartSquare)];
        this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, color, rookEndSquare)];
    }

    // update ep square
    char nextEp = -1;
    if(piece == Pawn && abs(from-to) == 16)
        nextEp = to + (color == White ? South : North);

    if(this->ep != -1) this->hashKey ^= zobristNumbers[zEpFileIndex + (this->ep & 7)];
    if(nextEp != -1) this->hashKey ^= zobristNumbers[zEpFileIndex + (nextEp & 7)];

    // switch turn
    this->hashKey ^= zobristNumbers[zBlackTurnIndex];
}

const char hashFExact = 0;
const char hashFAlpha = 1;
const char hashFBeta = 2;

struct hashElement {
    U64 key;
    short depth;
    char flags;
    int value;
    int best;
};

const int tableSize = (1 << 19);
const int valUnknown = -1e9;

hashElement hashTable[tableSize];

// get the best move from the tt
int retrieveBestMove() {
    int index = (board.hashKey & (tableSize-1));
    hashElement *h = &hashTable[index];

    if(h->key != board.hashKey) return noMove;

    return h->best;
}

// check if the stored hash element corresponds to the current position and if it was searched at a good enough depth
int ProbeHash(short depth, int alpha, int beta) {
    int index = (board.hashKey & (tableSize-1));
    hashElement *h = &hashTable[index];

    if(h->key == board.hashKey) {
        if(h->depth >= depth) {
            if(h->flags == hashFExact) return h->value;
            if(h->flags == hashFAlpha && h->value <= alpha) return alpha;
            if(h->flags == hashFBeta && h->value >= beta) return beta;
        }
    }
    return valUnknown;
}

// replace hashed element if replacement conditions are met
void RecordHash(short depth, int val, char hashF, int best) {
    if(timeOver) return;

    int index = (board.hashKey & (tableSize-1));
    hashElement *h = &hashTable[index];

    bool skip = false;
    if((h->key == board.hashKey) && (h->depth > depth)) skip = true; 
    if(hashF != hashFExact && h->flags == hashFExact) skip = true; 
    if(hashF == hashFExact && h->flags != hashFExact) skip = false;
    if(skip) return;

    h->key = board.hashKey;
    h->best = best;
    h->value = val;
    h->flags = hashF;
    h->depth = depth;
}

void showPV(short depth) {
    vector<int> moves;
    cout << "pv ";
    while(retrieveBestMove() != noMove && depth) {
        int m = retrieveBestMove();
        cout << moveToString(m) << ' ';
        board.makeMove(m);
        moves.push_back(m);
        depth--;
    }
    cout << '\n';
    while(moves.size()) {
        board.unmakeMove(moves.back());
        moves.pop_back();
    }
}

struct pawnHashElement {
    U64 bb;
    int eval;
};

pawnHashElement pawnHashTable[tableSize];

// get hashed value from map
int retrievePawnEval(U64 pawns) {
    int idx = (board.hashKey & (tableSize-1));

    if(pawnHashTable[idx].bb == pawns) {
        return pawnHashTable[idx].eval;
    }

    return valUnknown;
}

void recordPawnEval(U64 pawns, int eval) {
    int idx = (board.hashKey & (tableSize-1));

    // always replace
    if(pawnHashTable[idx].bb != pawns) {
        pawnHashTable[idx] = {pawns, eval};
    }
}

void clearTT() {
    hashElement newElement = {0, 0, 0, 0, noMove};
    pawnHashElement newPawnElement = {0, 0};
    for(int i = 0; i < tableSize; i++) {
        hashTable[i] = newElement;
        pawnHashTable[i] = newPawnElement;
    }
}