#include <unordered_map>

#include "Board.h"
#include "Moves.h"

// move information is stored in an int as follows:
// first 6 bits are the starting square, next 6 are the to square
// bit 12 is the color of the moved piece
// bits 13-15 are the piece that is being moved, bits 16-18 are the captured piece
// bits 19-21 are the promoted piece
// bits 22 and 23 are flags that indicate if the move is a castle or an en passant

const int CAPTURE_MASK = ((1 << 16) | (1 << 17) | (1 << 18));
const int PROM_MASK = ((1 << 19) | (1 << 20) | (1 << 21));
const int FROM_MASK = 63; // first 6 bits
const int TO_MASK = ((1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
const int PIECE_MASK = ((1 << 13) | (1 << 14) | (1 << 15));

int getMove(int from, int to, bool color, int fromPiece, int capturedPiece, int prom, bool castle, bool ep) {
    return (from | (to << 6) | ((int)(color) << 12) | (fromPiece << 13) | 
           (capturedPiece << 16) | (prom << 19) | ((int)(castle) << 22) | ((int)(ep) << 23));
}

bool isCastle(int move) {return (move & bits[22]);}
bool isEP(int move) {return (move & bits[23]);}
bool isCapture(int move) {return (move & CAPTURE_MASK);}
bool isPromotion(int move) {return (move & PROM_MASK);}

int getFromSq(int move) {return (move & FROM_MASK);}
int getToSq(int move) {return ((move & TO_MASK) >> 6);}
int getColor(int move) {return ((move & bits[12]) >> 9);}
int getPiece(int move) {return ((move & PIECE_MASK) >> 13);}
int getCapturedPiece(int move) {return ((move & CAPTURE_MASK) >> 16);}
int getPromotionPiece(int move) {return ((move & PROM_MASK) >> 19);}