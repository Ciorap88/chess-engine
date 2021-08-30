#include <bits/stdc++.h>

#include "Board.h"
#include "TranspositionTable.h"
#include "Search.h"
#include "MagicBitboards.h"

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

int zPieceSquareIndex(int piece, int color, int square) {
    int idx = (2*(piece-1) + (int)(color == Black))*64 + square;
    return idx;
}

// xor zobrist numbers corresponding to all features of the current position
U64 getZobristHashFromCurrPos() {
    U64 key = 0;
    for(int i = 0; i < 64; i++) {
        int color = (board.squares[i] & (Black | White));
        int piece = (board.squares[i] ^ color);

        key ^= zobristNumbers[zPieceSquareIndex(piece, color, i)];
    }

    key ^= zobristNumbers[zCastleRightsIndex + board.castleRights];
    if(board.turn == Black) key ^= zobristNumbers[zBlackTurnIndex];

    int epFile = board.ep % 8;
    key ^= zobristNumbers[zEpFileIndex + epFile];

    return key;
}

// update the hash key after making a move
void Board::updateHashKey(Move m) {
    if(m.from == -1) { // null move
        this->hashKey ^= zobristNumbers[zEpFileIndex + (this->ep & 7)];
        this->hashKey ^= zobristNumbers[zBlackTurnIndex];
        return;
    }

    int color = (this->squares[m.from] & (Black | White));
    int piece = (this->squares[m.from] ^ color);
    int otherColor = (color ^ (Black | White));
    int otherPiece = (m.capture ^ otherColor);
    int capturedPieceSquare = (m.ep ? (m.to + (color == White ? South : North)) : m.to);

    // update pieces
    this->hashKey ^= zobristNumbers[zPieceSquareIndex(piece, color, m.from)];
    if(m.capture) this->hashKey ^= zobristNumbers[zPieceSquareIndex(otherPiece, otherColor, capturedPieceSquare)];

    if(!m.prom) this->hashKey ^= zobristNumbers[zPieceSquareIndex(piece, color, m.to)];
    else this->hashKey ^= zobristNumbers[zPieceSquareIndex(m.prom, color, m.to)];

    // castle stuff
    int newCastleRights = this->castleRights;
    if(piece == King) {
        int mask = (color == White ? 12 : 3);
        newCastleRights &= mask;
    }
    if((newCastleRights & bits[1]) && (m.from == a1 || m.to == a1))
        newCastleRights ^= bits[1];
    if((newCastleRights & bits[0]) && (m.from == h1 || m.to == h1))
        newCastleRights ^= bits[0];
    if((newCastleRights & bits[3]) && (m.from == a8 || m.to == a8))
        newCastleRights ^= bits[3];
    if((newCastleRights & bits[2]) && (m.from == h8 || m.to == h8))
        newCastleRights ^= bits[2];

    this->hashKey ^= zobristNumbers[zCastleRightsIndex + this->castleRights];
    this->hashKey ^= zobristNumbers[zCastleRightsIndex + newCastleRights];

    if(m.castle) {
        int Rank = (m.to >> 3), File = (m.to & 7);
        int rookStartSquare = (Rank << 3) + (File == 6 ? 7 : 0);
        int rookEndSquare = (Rank << 3) + (File == 6 ? 5 : 3);

        this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, color, rookStartSquare)];
        this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, color, rookEndSquare)];
    }

    // update ep square
    int nextEp = -1;
    if(piece == Pawn && abs(m.from-m.to) == 16)
        nextEp = m.to + (color == White ? South : North);

    if(this->ep != -1) this->hashKey ^= zobristNumbers[zEpFileIndex + (this->ep & 7)];
    if(nextEp != -1) this->hashKey ^= zobristNumbers[zEpFileIndex + (nextEp & 7)];

    // switch turn
    this->hashKey ^= zobristNumbers[zBlackTurnIndex];
}

const int hashFExact = 0;
const int hashFAlpha = 1;
const int hashFBeta = 2;

struct hashElement {
    U64 key;
    int depth, flags, value;
    Move best;
};

const int tableSize = (1 << 19);
const int valUnknown = -1e9;

hashElement hashTable[tableSize];

void clearTT() {
    hashElement newElement = {0, 0, 0, 0, noMove};
    for(int i = 0; i < tableSize; i++) {
        hashTable[i] = newElement;
    }
}

// get the best move from the tt
Move retrieveBestMove() {
    int index = (board.hashKey & (tableSize-1));
    hashElement *h = &hashTable[index];

    if(h->key != board.hashKey) return noMove;

    return h->best;
}

// check if the stored hash element corresponds to the current position and if it was searched at a good enough depth
int ProbeHash(int depth, int alpha, int beta) {
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
void RecordHash(int depth, int val, int hashF, Move best) {
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

void showPV(int depth) {
    vector<Move> moves;
    cout << "pv ";
    while(retrieveBestMove().from != noMove.from && depth) {
        Move m = retrieveBestMove();
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
