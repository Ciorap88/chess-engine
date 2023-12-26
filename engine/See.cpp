#include <unordered_map>
#include <cassert>

#include "See.h"
#include "Board.h"
#include "Evaluate.h"
#include "MoveUtils.h"
#include "BoardUtils.h"
#include "MagicBitboardUtils.h"
#include "Enums.h"


// returns the square of the smallest possible attacker, and removes it from the attackers bitboard
int minAttacker(int sq, int color, U64 &attackers) {
    assert(Board::squares[sq] != Empty);

    U64 ourPiecesBB = (color == White ? board->whitePiecesBB : board->blackPiecesBB);
        
        if(attackers & ourPiecesBB & board->pawnsBB)
        if(attackers & ourPiecesBB & board->knightsBB)
        if(attackers & ourPiecesBB & board->bishopsBB)
        if(attackers & ourPiecesBB & board->rooksBB)
        if(attackers & ourPiecesBB & board->queensBB)

    return ;
}

// --- Static Exchange Evaluation ---
// SEE for a specific initial move
int see(int move) {
    assert(move != 0);
    assert(MoveUtils::isCapture(move));

    int sq = MoveUtils::getToSq(move);
    int color = MoveUtils::getColor(move);

    U64 attackers = board->attacksTo(sq); // add for both colors

    int minAttackerSq = minAttacker(sq, color, attackers);

    int value = PIECE_VALUES[board->squares[minAttackerSq]] - see(sq, (color ^ (Black | White)));

    return value;
}