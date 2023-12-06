#include "BoardUtils.h"
#include "Board.h"
#include "MoveUtils.h"
#include "Enums.h"

#include <string>
#include <cassert>
using namespace std;

U64 BoardUtils::bits[64], BoardUtils::filesBB[8], BoardUtils::ranksBB[8], BoardUtils::knightAttacksBB[64], BoardUtils::kingAttacksBB[64], BoardUtils::whitePawnAttacksBB[64], BoardUtils::blackPawnAttacksBB[64];
U64 BoardUtils::squaresNearWhiteKing[64], BoardUtils::squaresNearBlackKing[64];
U64 BoardUtils::lightSquaresBB, BoardUtils::darkSquaresBB;
U64 BoardUtils::bishopMasks[64], BoardUtils::rookMasks[64], BoardUtils::castleMask[4];

const int BoardUtils::castleStartSq[4] = {e1, e1, e8, e8};
const int BoardUtils::castleEndSq[4] = {g1, c1, g8, c8};

// functions that return the board shifted in a direction
U64 BoardUtils::eastOne(U64 bb) { return ((bb << 1) & (~filesBB[0])); }
U64 BoardUtils::westOne(U64 bb) { return ((bb >> 1) & (~filesBB[7])); }
U64 BoardUtils::northOne(U64 bb) { return (bb << 8); }
U64 BoardUtils::southOne(U64 bb) { return (bb >> 8); }

// returns the algebraic notation for a move
string BoardUtils::moveToString(int move) {
    int from = MoveUtils::MoveUtils::getFromSq(move);
    int to = MoveUtils::MoveUtils::getToSq(move);
    int prom = MoveUtils::MoveUtils::getPromotionPiece(move);

    if(move == MoveUtils::NO_MOVE) return "0000";

    string s;
    s += (from%8)+'a';
    s += (from/8)+'1';
    s += (to%8)+'a';
    s += (to/8)+'1';

    if(prom == Knight) s += 'n';
    if(prom == Bishop) s += 'b';
    if(prom == Rook) s += 'r';
    if(prom == Queen) s += 'q';

    return s;
}

// returns the name of the square in algebraic notation
string BoardUtils::square(int x) {
    assert(x >= 0 && x < 64);

    string s;
    s += ('a'+x%8);
    s += ('1'+x/8);
    return s;
}

// returns true if we can go in a specific direction from a square and not go outside the board
bool BoardUtils::isInBoard(int sq, int dir) {
    int file = (sq & 7);
    int rank = (sq >> 3);

    if(dir == north) return rank < 7;
    if(dir == south) return rank > 0;
    if(dir == east) return file < 7;
    if(dir == west) return file > 0;

    if(dir == northEast) return rank < 7 && file < 7;
    if(dir == southEast) return rank > 0 && file < 7;
    if(dir == northWest) return rank < 7 && file > 0;
    if(dir == southWest) return rank > 0 && file > 0;

    return false;
}

// returns the direction of the move if any, or 0 otherwise
int BoardUtils::direction(int from, int to) {
    int fromRank = (from >> 3), fromFile = (from & 7);
    int toRank = (to >> 3), toFile = (to & 7);

    if(fromRank == toRank)
        return (to > from ? east : west);

    if(fromFile == toFile)
        return (to > from ? north : south);

    if(fromRank-toRank == fromFile-toFile)
        return (to > from ? northEast : southWest);

    if(fromRank-toRank == toFile-fromFile)
        return (to > from ? northWest : southEast);

    return 0;
}

// piece attack patterns
U64 BoardUtils::pawnAttacks (U64 pawns, int color) {
    if(color == White) {
        U64 north = northOne(pawns);
        return (eastOne(north) | westOne(north));
    }
    U64 south = southOne(pawns);
    return (eastOne(south) | westOne(south));
}

U64 BoardUtils::knightAttacks(U64 knights) {
    U64 east, west, attacks;

    east = eastOne(knights);
    west = westOne(knights);
    attacks = (northOne(northOne(east | west)) | southOne(southOne(east | west)));

    east = eastOne(east);
    west = westOne(west);

    attacks |= (northOne(east | west) | southOne(east | west));

    return attacks;
}