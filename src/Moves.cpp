#include "Board.h"
#include "Moves.h"

// move information is stored in an int as follows:
// first 6 bits are the starting square, next 6 are the to square
// bit 12 is the color of the moved piece
// bits 13-15 are the piece that is being moved, bits 16-18 are the captured piece
// bits 19-21 are the promoted piece
// bits 22 and 23 are flags that indicate if the move is a castle or an en passant
int getMove(char from, char to, bool color, char fromPiece, char capturedPiece, char prom, bool castle, bool ep) {
    return ((int)(from) | ((int)(to) << 6) | ((int)(color) << 12) | ((int)(fromPiece) << 13) | 
           ((int)(capturedPiece) << 16) | ((int)(prom) << 19) | ((int)(castle) << 22) | ((int)(ep) << 23));
}

