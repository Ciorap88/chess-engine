#include <iostream>
#include <vector>
#include <unordered_map>

#include "Board.h"
#include "TranspositionTable.h"
#include "Search.h"
#include "MagicBitboards.h"
#include "Moves.h"

using namespace std;

typedef unsigned long long U64;

U64 pieceZobristNumbers[7][2][64];
U64 castleZobristNumbers[16];
U64 epZobristNumbers[8];
U64 blackTurnZobristNumber;

void generateZobristHashNumbers() {
    for(int pc = 0; pc < 7; pc++) {
        for(int c = 0; c < 2; c++) {
            for(int sq = 0; sq < 64; sq++) {
                pieceZobristNumbers[pc][c][sq] = randomULL();
            }
        }
    }
    for(int castle = 0; castle < 16; castle++) {
        castleZobristNumbers[castle] = randomULL();
    }
    for(int col = 0; col < 8; col++) {
        epZobristNumbers[col] = randomULL();
    }
    blackTurnZobristNumber = randomULL();
}

// xor zobrist numbers corresponding to all features of the current position
U64 getZobristHashFromCurrPos() {
    U64 key = 0;
    for(int i = 0; i < 64; i++) {
        if(board.squares[i] == Empty) continue;
        int color = (board.squares[i] & (Black | White));
        int piece = (board.squares[i] ^ color);

        key ^= pieceZobristNumbers[piece][(int)(color == White)][i];
    }

    key ^= castleZobristNumbers[board.castleRights];
    if(board.ep != -1) key ^= epZobristNumbers[board.ep % 8];
    if(board.turn == Black) key ^= blackTurnZobristNumber;

    return key;
}

// update the hash key after making a move
void Board::updateHashKey(int move) {
    if(move == NO_MOVE) { // null move
        if(this->ep != -1) this->hashKey ^= epZobristNumbers[this->ep % 8];
        this->hashKey ^= blackTurnZobristNumber;
        return;
    }

    // get move info
    int from = getFromSq(move);
    int to = getToSq(move);

    int color = getColor(move);
    int piece = getPiece(move);
    int otherColor = (color ^ 8);
    int otherPiece = getCapturedPiece(move);

    bool isMoveEP = isEP(move);
    bool isMoveCapture = isCapture(move);
    bool isMoveCastle = isCastle(move);
    int promotionPiece = getPromotionPiece(move);

    int capturedPieceSquare = (isMoveEP ? (to + (color == White ? south : north)) : to);

    // update pieces
    this->hashKey ^= pieceZobristNumbers[piece][(int)(color == White)][from];
    if(isMoveCapture) this->hashKey ^= pieceZobristNumbers[otherPiece][(int)(otherColor == White)][capturedPieceSquare];

    if(!promotionPiece) this->hashKey ^= pieceZobristNumbers[piece][(int)(color == White)][to];
    else this->hashKey ^= pieceZobristNumbers[promotionPiece][(int)(color == White)][to];

    // castle stuff
    int newCastleRights = this->castleRights;
    if(piece == King) {
        int mask = (color == White ? 12 : 3);
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

    this->hashKey ^= castleZobristNumbers[this->castleRights];
    this->hashKey ^= castleZobristNumbers[newCastleRights];

    if(isMoveCastle) {
        int Rank = (to >> 3), File = (to & 7);
        int rookStartSquare = (Rank << 3) + (File == 6 ? 7 : 0);
        int rookEndSquare = (Rank << 3) + (File == 6 ? 5 : 3);

        this->hashKey ^= pieceZobristNumbers[Rook][(int)(color == White)][rookStartSquare];
        this->hashKey ^= pieceZobristNumbers[Rook][(int)(color == White)][rookEndSquare];
    }

    // update ep square
    int nextEp = -1;
    if(piece == Pawn && abs(from-to) == 16) {
        nextEp = to + (color == White ? south : north);
    }

    if(this->ep != -1) this->hashKey ^= epZobristNumbers[this->ep % 8];
    if(nextEp != -1) this->hashKey ^= epZobristNumbers[nextEp % 8];

    // switch turn
    this->hashKey ^= blackTurnZobristNumber;
}

const int HASH_F_EXACT = 0;
const int HASH_F_ALPHA = 1;
const int HASH_F_BETA = 2;

struct hashElement {
    U64 key;
    short depth;
    int flags;
    int value;
    int best;
};


const int TABLE_SIZE = (1 << 25);
const int VAL_UNKNOWN = -1e9;


hashElement hashTable[TABLE_SIZE];

// get the best move from the tt
int retrieveBestMove() {
    int index = (board.hashKey & (TABLE_SIZE-1));
    hashElement *h = &hashTable[index];

    if(h->key != board.hashKey) return NO_MOVE;

    return h->best;
}

// check if the stored hash element corresponds to the current position and if it was searched at a good enough depth
int probeHash(short depth, int alpha, int beta) {
    int index = (board.hashKey & (TABLE_SIZE-1));
    hashElement *h = &hashTable[index];

    if(h->key == board.hashKey) {
        if(h->depth >= depth) {
            if((h->flags == HASH_F_EXACT || h->flags == HASH_F_ALPHA) && h->value <= alpha) return alpha;
            if((h->flags == HASH_F_EXACT || h->flags == HASH_F_BETA) && h->value >= beta) return beta;
            if(h->flags == HASH_F_EXACT && h->value > alpha && h->value < beta) return h->value;
        }
    }
    return VAL_UNKNOWN;
}

// replace hashed element if replacement conditions are met
void recordHash(short depth, int val, int hashF, int best) {
    if(timeOver) return;

    int index = (board.hashKey & (TABLE_SIZE-1));
    hashElement *h = &hashTable[index];

    bool skip = false;
    if((h->key == board.hashKey) && (h->depth > depth)) skip = true; 
    if(hashF != HASH_F_EXACT && h->flags == HASH_F_EXACT) skip = true; 
    if(hashF == HASH_F_EXACT && h->flags != HASH_F_EXACT) skip = false;
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
    while(retrieveBestMove() != NO_MOVE && depth) {
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

pawnHashElement pawnHashTable[TABLE_SIZE];

// get hashed value from map
int retrievePawnEval(U64 pawns) {
    int idx = (board.hashKey & (TABLE_SIZE-1));

    if(pawnHashTable[idx].bb == pawns) {
        return pawnHashTable[idx].eval;
    }

    return VAL_UNKNOWN;
}

void recordPawnEval(U64 pawns, int eval) {
    int idx = (board.hashKey & (TABLE_SIZE-1));

    // always replace
    if(pawnHashTable[idx].bb != pawns) {
        pawnHashTable[idx] = {pawns, eval};
    }
}

void clearTT() {
    hashElement newElement = {0, 0, 0, 0, NO_MOVE};
    pawnHashElement newPawnElement = {0, 0};
    for(int i = 0; i < TABLE_SIZE; i++) {
        hashTable[i] = newElement;
        pawnHashTable[i] = newPawnElement;
    }
}