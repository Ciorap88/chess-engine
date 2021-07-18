#include <bits/stdc++.h>

#include "Board.h"
#include "MagicBitboards.h"

using namespace std;

U64 bits[64];
U64 filesBB[8], ranksBB[8], knightAttacksBB[64], kingAttacksBB[64], whitePawnAttacksBB[64], blackPawnAttacksBB[64];
U64 squaresNearWhiteKing[64], squaresNearBlackKing[64];
U64 lightSquaresBB, darkSquaresBB;
U64 castleMask[4];
U64 bishopMasks[64], rookMasks[64];

int castleStartSq[4] = {e1,e1,e8,e8};
int castleEndSq[4] = {g1,c1,g8,c8};

enum Directions {
    North = 8,
    South = -8,
    East = 1,
    West = -1,

    SouthEast = South+East,
    SouthWest = South+West,
    NorthEast = North+East,
    NorthWest = North+West
};

// index of least significant set bit
int bitscanForward(U64 bb) {
    return __builtin_ctzll(bb);
}

string moveToString(Move m) {
    string s;
    s += (m.from%8)+'a';
    s += (m.from/8)+'1';
    s += (m.to%8)+'a';
    s += (m.to/8)+'1';

    if(m.prom == Knight) s += 'n';
    if(m.prom == Bishop) s += 'b';
    if(m.prom == Rook) s += 'r';
    if(m.prom == Queen) s += 'q';

    return s;
}

string square(int x) {
    string s;
    s += ('a'+x%8);
    s += ('1'+x/8);
    return s;
}

// general bitboard operations
int popcount(U64 bb) {
    int res;
    for(res = 0; bb; res++, bb &= bb - 1);
    return res;
}

U64 eastOne(U64 bb) {
    return ((bb << 1) & (~filesBB[0]));
}

U64 westOne(U64 bb) {
    return ((bb >> 1) & (~filesBB[7]));
}

U64 northOne(U64 bb) {
    return (bb << 8);
}
U64 southOne(U64 bb) {
    return (bb >> 8);
}

bool isInBoard(int sq, int dir) {
    int File = (sq & 7);
    int Rank = (sq >> 3);

    if(dir == North) return Rank < 7;
    if(dir == South) return Rank > 0;
    if(dir == East) return File < 7;
    if(dir == West) return File > 0;

    if(dir == NorthEast) return Rank < 7 && File < 7;
    if(dir == SouthEast) return Rank > 0 && File < 7;
    if(dir == NorthWest) return Rank < 7 && File > 0;
    if(dir == SouthWest) return Rank > 0 && File > 0;

    return false;
}


// indices in zobristNumbers vector
const int zBlackTurnIndex = 12*64;
const int zCastleRightsIndex = 12*64+1;
const int zEpFileIndex = 12*64+17;

vector<U64> zobristNumbers;

U64 randomULL() {
    static U64 next = 1;

    next = next * 1103515245 + 12345;
    return next;
}

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
void Board::initZobristHashFromCurrPos() {
    this->hashKey = 0;
    for(int i = 0; i < 64; i++) {
        int color = (this->squares[i] & (Black | White));
        int piece = (this->squares[i] ^ color);

        this->hashKey ^= zobristNumbers[zPieceSquareIndex(piece, color, i)];
    }

    this->hashKey ^= zobristNumbers[zCastleRightsIndex + this->castleRights];
    if(this->turn == Black) this->hashKey ^= zobristNumbers[zBlackTurnIndex];

    int epFile = this->ep % 8;
    this->hashKey ^= zobristNumbers[zEpFileIndex + epFile];
}


void Init() {
    // bitmasks for every bit from 0 to 63
    for(int i = 0; i < 64; i++)
        bits[i] = (1LL << i);

    // bitboard for checking empty squares between king and rook when castling
    castleMask[0] = (bits[f1] | bits[g1]);
    castleMask[1] = (bits[b1] | bits[c1] | bits[d1]);
    castleMask[2] = (bits[f8] | bits[g8]);
    castleMask[3] = (bits[b8] | bits[c8] | bits[d8]);

    // initialize zobrist numbers in order to make zobrist hash keys
    generateZobristHashNumbers();

    // create file and rank masks
    for(int i = 0; i < 64; i++) {
        int File = (i & 7), Rank = (i >> 3);

        filesBB[File] |= bits[i];
        ranksBB[Rank] |= bits[i];
    }

    // create pawn attacks masks and vectors
    for(int i = 0; i < 64; i++) {
        int File = (i & 7), Rank = (i >> 3);

        if(File > 0) {
            if(i+7 < 64) whitePawnAttacksBB[i] |= bits[i+7];
            if(i-9 >= 0) blackPawnAttacksBB[i] |= bits[i-9];
        }
        if(File < 7) {
            if(i+9 < 64) whitePawnAttacksBB[i] |= bits[i+9];
            if(i-7 >= 0) blackPawnAttacksBB[i] |= bits[i-7];
        }
    }

    // create knight attacks masks and vectors
    for(int i = 0; i < 64; i++) {
        int File = (i & 7), Rank = (i >> 3);

        if(File > 1) {
            if(Rank > 0) knightAttacksBB[i] |= bits[i-10];
            if(Rank < 7) knightAttacksBB[i] |= bits[i+6];
        }
        if(File < 6) {
            if(Rank > 0) knightAttacksBB[i] |= bits[i-6];
            if(Rank < 7) knightAttacksBB[i] |= bits[i+10];
        }
        if(Rank > 1) {
            if(File > 0) knightAttacksBB[i] |= bits[i-17];
            if(File < 7) knightAttacksBB[i] |= bits[i-15];
        }
        if(Rank < 6) {
            if(File > 0) knightAttacksBB[i] |= bits[i+15];
            if(File < 7) knightAttacksBB[i] |= bits[i+17];
        }
    }

    // create bishop masks
    for(int i = 0; i < 64; i++) {
        for(auto dir: {NorthEast, NorthWest, SouthEast, SouthWest}) {
            int sq = i;
            while(isInBoard(sq, dir)) {
                bishopMasks[i] |= bits[sq];
                sq += dir;
            }
        }
        bishopMasks[i] ^= bits[i];
    }

    // create rook masks
    for(int i = 0; i < 64; i++) {
        for(auto dir: {East, West, North, South}) {
            int sq = i;
            while(isInBoard(sq, dir)) {
                rookMasks[i] |= bits[sq];
                sq += dir;
            }
        }
        rookMasks[i] ^= bits[i];
    }

    // create king moves masks and vectors
    for(int i = 0; i < 64; i++) {
        kingAttacksBB[i] = (eastOne(bits[i]) | westOne(bits[i]));
        U64 king = (bits[i] | kingAttacksBB[i]);
        kingAttacksBB[i] |= (northOne(king) | southOne(king));
    }

    // squares near king are squares that a king can move to and the squares in front of his 'forward' moves
    for(int i = 0; i < 64; i++) {
        squaresNearWhiteKing[i] = squaresNearBlackKing[i] = (kingAttacksBB[i] | bits[i]);
        if(i+South >= 0) squaresNearBlackKing[i] |= kingAttacksBB[i+South];
        if(i+North < 64) squaresNearWhiteKing[i] |= kingAttacksBB[i+North];
    }

    // create light and dark squares masks
    for(int i = 0; i < 64; i++) {
        if(i%2) lightSquaresBB |= bits[i];
        else darkSquaresBB |= bits[i];
    }

    initMagics();
}

// returns the direction of the move if any, or 0 otherwise
int direction(int from, int to) {
    int fromRank = (from >> 3), fromFile = (from & 7);
    int toRank = (to >> 3), toFile = (to & 7);

    if(fromRank == toRank)
        return (to > from ? East : West);

    if(fromFile == toFile)
        return (to > from ? North : South);

    if(fromRank-toRank == fromFile-toFile)
        return (to > from ? NorthEast : SouthWest);

    if(fromRank-toRank == toFile-fromFile)
        return (to > from ? NorthWest : SouthEast);

    return 0;
}

// piece attack patterns
U64 pawnAttacks (U64 pawns, int color) {
    if(color == White) {
        U64 north = northOne(pawns);
        return (eastOne(north) | westOne(north));
    }
    U64 south = southOne(pawns);
    return (eastOne(south) | westOne(south));
}

U64 knightAttacks(U64 knights) {
    U64 east, west, attacks;

    east = eastOne(knights);
    west = westOne(knights);
    attacks = (northOne(northOne(east | west)) | southOne(southOne(east | west)));

    east = eastOne(east);
    west = westOne(west);

    attacks |= (northOne(east | west) | southOne(east | west));

    return attacks;
}

void Board::updatePieceInBB(int piece, int color, int sq) {
    if(color == White) this->whitePiecesBB ^= bits[sq];
    else this->blackPiecesBB ^= bits[sq];

    if(piece == Pawn) this->pawnsBB ^= bits[sq];
    if(piece == Knight) this->knightsBB ^= bits[sq];
    if(piece == Bishop) this->bishopsBB ^= bits[sq];
    if(piece == Rook) this->rooksBB ^= bits[sq];
    if(piece == Queen) this->queensBB ^= bits[sq];
}

void Board::movePieceInBB(int piece, int color, int from, int to) {
    this->updatePieceInBB(piece, color, from);
    this->updatePieceInBB(piece, color, to);
}

string Board::getFenFromCurrPos() {
    unordered_map<int, char> pieceSymbols = {{Pawn, 'p'}, {Knight, 'n'},
    {Bishop, 'b'}, {Rook, 'r'}, {Queen, 'q'}, {King, 'k'}};

    string fen;

    int emptySquares = 0;
    for(int Rank = 7; Rank >= 0; Rank--)
        for(int File = 0; File < 8; File++) {
            int color = (this->squares[Rank*8 + File] & (Black | White));
            int piece = (this->squares[Rank*8 + File] ^ color);

            if(this->squares[Rank*8 + File] == Empty) {
                emptySquares++;
            } else {
                if(emptySquares) {
                    fen += (emptySquares + '0');
                    emptySquares = 0;
                }
                fen += (color == White ? toupper(pieceSymbols[piece]) : pieceSymbols[piece]);
            }

            if(File == 7) {
                if(emptySquares) fen += (emptySquares+'0');
                emptySquares = 0;

                if(Rank > 0) fen += '/';
            }
        }

    fen += (this->turn == White ? " w " : " b ");

    string castles;
    if(this->castleRights & bits[0]) castles += 'K';
    if(this->castleRights & bits[1]) castles += 'Q';
    if(this->castleRights & bits[2]) castles += 'k';
    if(this->castleRights & bits[3]) castles += 'q';
    if(castles.length() == 0) castles += '-';

    fen += castles;
    fen += ' ';

    if(this->ep == -1) fen += '-';
    else fen += square(this->ep);

    // add halfmove clock and fullmove number when I need them

    return fen;
}

void Board::loadFenPos(string pieces, char turn, string castles, string epTargetSq, int halfMoveClock, int fullMoveNumber) {
    this->blackPiecesBB = this->whitePiecesBB = 0;
    this->pawnsBB = this->knightsBB = 0;
    this->bishopsBB = this->rooksBB = 0;
    this->queensBB = 0;

    unordered_map<char, int> pieceSymbols = {{'p', Pawn}, {'n', Knight},
    {'b', Bishop}, {'r', Rook}, {'q', Queen}, {'k', King}};

    int File = 0, Rank = 7;
    for(char p: pieces) {
        if(p == '/') {
            Rank--;
            File = 0;
        } else {
            // if it is a digit skip the squares
            if(p-'0' >= 1 && p-'0' <= 8) {
                for(int i = 0; i < (p-'0'); i++) {
                    this->squares[Rank*8 + File] = 0;
                    File++;
                }
            } else {
                int color = ((p >= 'A' && p <= 'Z') ? White : Black);
                int type = pieceSymbols[tolower(p)];

                int currBit = bits[Rank*8 + File];

                this->updatePieceInBB(type, color, Rank*8 + File);
                if(type == King) {
                    if(color == White) this->whiteKingSquare = Rank*8 + File;
                    if(color == Black) this->blackKingSquare = Rank*8 + File;
                }

                this->squares[Rank*8 + File] = (color | type);
                File++;
            }
        }
    }

    // get turn
    this->turn = (turn == 'w' ? White : Black);

    // get castling rights
    unordered_map<char, U64> castleSymbols = {{'K', bits[0]}, {'Q', bits[1]}, {'k', bits[2]}, {'q', bits[3]}, {'-', 0}};
    this->castleRights = 0;

    for(char c: castles)
        this->castleRights |= castleSymbols[c];

    // get en passant target square
    this->ep = (epTargetSq == "-" ? -1 : (epTargetSq[0]-'a' + 8*(epTargetSq[1]-'1')));

    // initialize hash key
    initZobristHashFromCurrPos();
}

vector<Move> Board::GeneratePseudoLegalMoves() {
    int color = this->turn;
    int otherColor = (color ^ (Black | White));

    U64 ourPiecesBB = (color == White ? this->whitePiecesBB : this->blackPiecesBB);
    U64 opponentPiecesBB = (color == White ? this->blackPiecesBB : this->whitePiecesBB);
    U64 allPiecesBB = (this->whitePiecesBB | this->blackPiecesBB);

    vector<Move> moves;

    // -----pawns-----
    U64 ourPawnsBB = (ourPiecesBB & this->pawnsBB);
    int pawnDir = (color == White ? North : South);
    int pawnStartRank = (color == White ? 1 : 6);
    int pawnPromRank = (color == White ? 7 : 0);

    while(ourPawnsBB) {
        int sq = bitscanForward(ourPawnsBB);
        bool isPromoting = (((sq+pawnDir) >> 3) == pawnPromRank);

        // normal moves and promotions
        if((allPiecesBB & bits[sq+pawnDir]) == 0) {

            if((sq >> 3) == pawnStartRank && (allPiecesBB & bits[sq+2*pawnDir]) == 0)
                moves.push_back({sq, sq+2*pawnDir, 0, 0, 0, 0}); // 2 square move

            if(isPromoting) {
                for(int piece: {Knight, Bishop, Rook, Queen})
                    moves.push_back({sq, sq+pawnDir, 0, 0, 0, piece});
            } else moves.push_back({sq, sq+pawnDir, 0, 0, 0, 0});
        }

        // captures and capture-promotions
        U64 pawnCapturesBB = (color == White ? whitePawnAttacksBB[sq] : blackPawnAttacksBB[sq]);
        pawnCapturesBB &= opponentPiecesBB;
        while(pawnCapturesBB) {
            int to = bitscanForward(pawnCapturesBB);
            if(isPromoting) {
                for(int piece: {Knight, Bishop, Rook, Queen})
                    moves.push_back({sq, to, this->squares[to], 0, 0, piece});
            } else moves.push_back({sq, to, this->squares[to], 0, 0 ,0});

            pawnCapturesBB &= (pawnCapturesBB-1);

        }

        ourPawnsBB &= (ourPawnsBB-1);
    }

    //-----knights-----
    U64 ourKnightsBB = (this->knightsBB & ourPiecesBB);

    while(ourKnightsBB) {
        int sq = bitscanForward(ourKnightsBB);

        U64 knightMoves = (knightAttacksBB[sq] & ~ourPiecesBB);
        while(knightMoves) {
            int to = bitscanForward(knightMoves);
            moves.push_back({sq, to, this->squares[to], 0, 0, 0});
            knightMoves &= (knightMoves-1);
        }

        ourKnightsBB &= (ourKnightsBB-1);
    }

    //-----king-----
    int kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);
    U64 ourKingMoves = (kingAttacksBB[kingSquare] & ~ourPiecesBB);

    while(ourKingMoves) {
        int to = bitscanForward(ourKingMoves);

        moves.push_back({kingSquare, to, this->squares[to], 0, 0, 0});

        ourKingMoves &= (ourKingMoves-1);
    }

    //-----sliding pieces-----
    U64 rooksQueens = (ourPiecesBB & (this->rooksBB | this->queensBB));
    while(rooksQueens) {
        int sq = bitscanForward(rooksQueens);

        U64 rookMoves = (magicRookAttacks(allPiecesBB, sq) & ~ourPiecesBB);
        while(rookMoves) {
            int to = bitscanForward(rookMoves);

            moves.push_back({sq, to, this->squares[to], 0, 0, 0});

            rookMoves &= (rookMoves-1);
        }

        rooksQueens &= (rooksQueens-1);
    }

    U64 bishopsQueens = (ourPiecesBB & (this->bishopsBB | this->queensBB));
    while(bishopsQueens) {
        int sq = bitscanForward(bishopsQueens);

        U64 bishopMoves = (magicBishopAttacks(allPiecesBB, sq) & ~ourPiecesBB);
        while(bishopMoves) {
            int to = bitscanForward(bishopMoves);

            moves.push_back({sq, to, this->squares[to], 0, 0, 0});

            bishopMoves &= (bishopMoves-1);
        }

        bishopsQueens &= (bishopsQueens-1);
    }

    // -----castles-----
    for(int i = 0; i < 4; i++)
        if((this->castleRights & bits[i]) && ((allPiecesBB & castleMask[i]) == 0))
             moves.push_back({castleStartSq[i], castleEndSq[i], 0, 0, 1, 0});

    // -----en passant-----
    if(ep != -1) {
        ourPawnsBB = (this->pawnsBB & ourPiecesBB);
        U64 epBB = ((color == White ? blackPawnAttacksBB[ep] : whitePawnAttacksBB[ep]) & ourPawnsBB);
        while(epBB) {
            int sq = bitscanForward(epBB);

            moves.push_back({sq, ep, (Pawn | otherColor), 1, 0, 0});

            epBB &= (epBB-1);
        }
    }
    return moves;
}

bool Board::isAttacked(int sq) {
    int color = this->turn;
    int otherColor = (color ^ (Black | White));
    int otherKingSquare = (otherColor == White ? this->whiteKingSquare : this->blackKingSquare);

    U64 opponentPiecesBB = (color == White ? this->blackPiecesBB : this->whitePiecesBB);
    U64 allPiecesBB = (this->whitePiecesBB | this->blackPiecesBB);
    U64 bishopsQueens = (opponentPiecesBB & (this->bishopsBB | this->queensBB));
    U64 rooksQueens = (opponentPiecesBB & (this->rooksBB | this->queensBB));

    // king attacks
    if(kingAttacksBB[otherKingSquare] & bits[sq])
        return true;

    // knight attacks
    if(knightAttacksBB[sq] & opponentPiecesBB & this->knightsBB)
        return true;

    // pawn attacks
    if(bits[sq] & pawnAttacks((opponentPiecesBB & this->pawnsBB), otherColor))
        return true;

    // sliding piece attacks
    if(bishopsQueens & magicBishopAttacks(allPiecesBB, sq))
        return true;

    if(rooksQueens & magicRookAttacks(allPiecesBB, sq))
        return true;

    return false;
}

bool Board::isInCheck() {
    int color = this->turn;
    int kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);
    return this->isAttacked(kingSquare);
}

U64 Board::attacksTo(int sq) {
    int color = (this->turn ^ (Black | White));

    U64 ourPiecesBB = (color == White ? this->whitePiecesBB : this->blackPiecesBB);
    U64 allPiecesBB = (this->whitePiecesBB | this->blackPiecesBB);
    U64 pawnAtt = (color == Black ? whitePawnAttacksBB[sq] : blackPawnAttacksBB[sq]);
    U64 rooksQueens = (ourPiecesBB & (this->rooksBB | this->queensBB));
    U64 bishopsQueens = (ourPiecesBB & (this->bishopsBB | this->queensBB));
    int kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);

    U64 res = 0;
    res |= (knightAttacksBB[sq] & ourPiecesBB & this->knightsBB);
    res |= (pawnAtt & this->pawnsBB & ourPiecesBB);
    res |= (kingAttacksBB[sq] & bits[kingSquare]);
    res |= (magicBishopAttacks(allPiecesBB, sq) & bishopsQueens);
    res |= (magicRookAttacks(allPiecesBB, sq) & rooksQueens);

    return res;
}

vector<Move> Board::GenerateLegalMoves() {
    int color = this->turn;
    int otherColor = (color ^ (Black | White));
    int kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);
    int otherKingSquare = (otherColor == White ? this->whiteKingSquare : this->blackKingSquare);

    U64 allPiecesBB = (this->whitePiecesBB | this->blackPiecesBB);
    U64 opponentPiecesBB = (color == White ? this->blackPiecesBB : this->whitePiecesBB);
    U64 ourPiecesBB = (color == White ? this->whitePiecesBB : this->blackPiecesBB);

    vector<Move> potentialMoves = this->GeneratePseudoLegalMoves();

    // remove the king so we can correctly find all squares attacked by sliding pieces, where the king can't go
    if(color == White) this->whitePiecesBB ^= bits[kingSquare];
    else this->blackPiecesBB ^= bits[kingSquare];

    U64 checkingPiecesBB = this->attacksTo(kingSquare);
    int checkingPiecesCnt = popcount(checkingPiecesBB);
    int checkingPieceIndex = bitscanForward(checkingPiecesBB);

    // check ray is the path between the king and the sliding piece checking it
    U64 checkRayBB = 0;
    if(checkingPiecesCnt == 1) {
        checkRayBB |= checkingPiecesBB;
        int piece = (this->squares[checkingPieceIndex]^otherColor);
        int dir = direction(checkingPieceIndex, kingSquare);

        // check direction is a straight line
        if(abs(dir) == North || abs(dir) == East)
            checkRayBB |= (magicRookAttacks(allPiecesBB, kingSquare) & magicRookAttacks(allPiecesBB, checkingPieceIndex));

        // check direction is a diagonal
        else checkRayBB |= (magicBishopAttacks(allPiecesBB, kingSquare) & magicBishopAttacks(allPiecesBB, checkingPieceIndex));
    }

    // finding the absolute pins
    U64 pinnedBB = 0;

    U64 attacks = magicRookAttacks(allPiecesBB, kingSquare); // attacks on the rook directions from the king square to our pieces
    U64 blockers = (ourPiecesBB & attacks); // potentially pinned pieces
    U64 oppRooksQueens = ((this->rooksBB | this->queensBB) & opponentPiecesBB);
    U64 pinners = ((attacks ^ magicRookAttacks((allPiecesBB ^ blockers), kingSquare)) & oppRooksQueens); // get pinners by computing attacks on the board without the blockers
    while(pinners) {
        int sq = bitscanForward(pinners);

        // get pinned pieces by &-ing attacks from the rook square with attacks from the king square, and then with our own pieces
        pinnedBB |= (magicRookAttacks(allPiecesBB, sq) & magicRookAttacks(allPiecesBB, kingSquare) & ourPiecesBB);
        pinners &= (pinners-1); // remove bit
    }

    attacks = magicBishopAttacks(allPiecesBB, kingSquare);
    blockers = (ourPiecesBB & attacks);
    U64 oppBishopsQueens = ((this->bishopsBB | this->queensBB) & opponentPiecesBB);
    pinners = ((attacks ^ magicBishopAttacks((allPiecesBB ^ blockers), kingSquare)) & oppBishopsQueens);
    while(pinners) {
        int sq = bitscanForward(pinners);
        pinnedBB |= (magicBishopAttacks(allPiecesBB, sq) & magicBishopAttacks(allPiecesBB, kingSquare) & ourPiecesBB);
        pinners &= (pinners-1);
    }

    vector<Move> moves;
    for(Move m: potentialMoves) {
        // we can always move the king to a safe square
        if(m.from == kingSquare) {
            if(this->isAttacked(m.to) == false) moves.push_back(m);
            continue;
        }

        // single check, can capture the attacker or intercept the check only if the moving piece is not pinned
        if(checkingPiecesCnt == 1 && ((pinnedBB & bits[m.from]) == 0)) {

            // capturing the checking pawn by en passant (special case)
            if(m.ep && checkingPieceIndex == m.to + (color == White ? South: North))
                moves.push_back(m);

            // check ray includes interception or capturing the attacker
            else if(checkRayBB & bits[m.to]) {
                moves.push_back(m);
            }
        }

        // no checks, every piece can move if it is not pinned or it moves in the direction of the pin
        if(checkingPiecesCnt == 0) {
            // not pinned
            if((pinnedBB & bits[m.from]) == 0)
                moves.push_back(m);

            // pinned, can only move in the pin direction
            else if(abs(direction(m.from, m.to)) == abs(direction(m.from, kingSquare)))
                moves.push_back(m);
        }
    }

    // en passant are the last added pseudo legal moves so we know they are at the back of the vector if they exist
    vector<Move> epMoves;
    while(moves.size() && moves.back().ep) {
        epMoves.push_back(moves.back());
        moves.pop_back();
    }

    // before ep there are the castle moves so we do the same
    vector<Move> castleMoves;
    while(moves.size() && moves.back().castle) {
        castleMoves.push_back(moves.back());
        moves.pop_back();
    }

    // manual check for the legality of castle moves
    for(Move m: castleMoves) {
        int first = min(m.from, m.to);
        int second = max(m.from, m.to);

        bool ok = true;

        for(int i = first; i <= second; i++)
            if(this->isAttacked(i))
                ok = false;

        if(ok) moves.push_back(m);
    }

    // ep horizontal pin
    for(Move enPassant: epMoves) {
        int epRank = (enPassant.from >> 3);
        int otherPawnSquare = (epRank << 3) | (enPassant.to & 7);
        U64 rooksQueens = (opponentPiecesBB & (this->rooksBB | this->queensBB) & ranksBB[epRank]);

        // remove the 2 pawns and compute attacks from rooks/queens on king square
        U64 removeWhite = bits[enPassant.from], removeBlack = bits[otherPawnSquare];
        if(color == Black) swap(removeWhite, removeBlack);

        this->whitePiecesBB ^= removeWhite;
        this->blackPiecesBB ^= removeBlack;

        U64 attacks = this->attacksTo(kingSquare);
        if((rooksQueens & attacks) == 0) moves.push_back(enPassant);

        this->whitePiecesBB ^= removeWhite;
        this->blackPiecesBB ^= removeBlack;
    }

    // put the king back
    if(color == White) this->whitePiecesBB ^= bits[kingSquare];
    else this->blackPiecesBB ^= bits[kingSquare];

    return moves;
}

void Board::updateHashKey(Move m) {
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

void Board::makeMove(Move m) {
    int color = (this->squares[m.from] & (Black | White));
    int piece = (this->squares[m.from] ^ color);
    int otherColor = (color ^ (Black | White));
    int otherPiece = (m.capture ^ otherColor);

    this->updateHashKey(m);

    // update bitboards
    this->updatePieceInBB(piece, color, m.from);
    if(!m.ep && m.capture) this->updatePieceInBB(otherPiece, otherColor, m.to);
    if(!m.prom) this->updatePieceInBB(piece, color, m.to);

    this->squares[m.to] = this->squares[m.from];
    this->squares[m.from] = Empty;

    // promote pawn
    if(m.prom) {
        this->updatePieceInBB(m.prom, color, m.to);
        this->squares[m.to] = (m.prom | color);
    }

    if(piece == King) {
        // bit mask for removing castle rights
        int mask = (color == White ? 12 : 3);
        this->castleRights &= mask;

        if(color == White) this->whiteKingSquare = m.to;
        else this->blackKingSquare = m.to;
    }

    // remove the respective castling right if a rook moves or gets captured
    if((this->castleRights & bits[1]) && (m.from == a1 || m.to == a1))
        this->castleRights ^= bits[1];
    if((this->castleRights & bits[0]) && (m.from == h1 || m.to == h1))
        this->castleRights ^= bits[0];
    if((this->castleRights & bits[3]) && (m.from == a8 || m.to == a8))
        this->castleRights ^= bits[3];
    if((this->castleRights & bits[2]) && (m.from == h8 || m.to == h8))
        this->castleRights ^= bits[2];

    // move the rook if castle
    if(m.castle) {
        int Rank = (m.to >> 3), File = (m.to & 7);
        int rookStartSquare = (Rank << 3) + (File == 6 ? 7 : 0);
        int rookEndSquare = (Rank << 3) + (File == 6 ? 5 : 3);

        this->movePieceInBB(Rook, color, rookStartSquare, rookEndSquare);
        swap(this->squares[rookStartSquare], this->squares[rookEndSquare]);
    }
    // remove the captured pawn if en passant
    if(m.ep) {
        int capturedPawnSquare = m.to+(color == White ? South : North);

        this->updatePieceInBB(Pawn, otherColor, capturedPawnSquare);
        this->squares[capturedPawnSquare] = Empty;
    }

    this->ep = -1;
    if(piece == Pawn && abs(m.from-m.to) == 16)
        this->ep = m.to+(color == White ? -8 : 8);

    // switch turn
    this->turn ^= (Black | White);
}

// basically the inverse of makeMove but we need to memorize the castling right and ep square before the move
void Board::unmakeMove(Move m, int ep, int castleRights) {
    int color = (this->squares[m.to] & (Black | White));
    int otherColor = (color ^ (Black | White));
    int piece = (this->squares[m.to] ^ color);
    int otherPiece = (m.capture ^ otherColor);

    this->ep = ep;
    this->castleRights = castleRights;

    if(piece == King) {
        if(color == White) this->whiteKingSquare = m.from;
        if(color == Black) this->blackKingSquare = m.from;
    }

    this->updatePieceInBB(piece, color, m.to);

    if(m.prom) this->squares[m.to] = (Pawn | color);

    this->updatePieceInBB((this->squares[m.to] ^ color), color, m.from);
    if(m.capture && !m.ep) this->updatePieceInBB(otherPiece, otherColor, m.to);

    this->squares[m.from] = this->squares[m.to];
    this->squares[m.to] = m.capture;

    if(m.castle) {
        int Rank = (m.to >> 3), File = (m.to & 7);
        int rookStartSquare = (Rank << 3) + (File == 6 ? 7 : 0);
        int rookEndSquare = (Rank << 3) + (File == 6 ? 5 : 3);

        this->movePieceInBB(Rook, color, rookEndSquare, rookStartSquare);
        swap(this->squares[rookStartSquare], this->squares[rookEndSquare]);
    }

    if(m.ep) {
        int capturedPawnSquare = m.to+(color == White ? South : North);

        this->updatePieceInBB(Pawn, otherColor, capturedPawnSquare);
        this->squares[capturedPawnSquare] = (otherColor | Pawn);
        this->squares[m.to] = Empty;
    }

    this->turn ^= (Black | White);

    this->updateHashKey(m);
}

Board board;

// returns true if the move puts the opponent's king in check
bool putsKingInCheck(Move a) {
    bool check = false;

    int ep = board.ep;
    int castleRights = board.castleRights;

    board.makeMove(a);
    if(board.isInCheck()) check = true;
    board.unmakeMove(a, ep, castleRights);

    return check;
}

// perft function that returns the number of positions reached from an initial position after a certain depth
ofstream q("perft.txt");
int moveGenTest(int depth, bool show) {
    if(depth == 0) return 1;

    vector<Move> moves = board.GenerateLegalMoves();
    int numPos = 0;

    for(Move m: moves) {
        int ep = board.ep;
        int castleRights = board.castleRights;

        board.makeMove(m);
        int mv = moveGenTest(depth-1, false);

        if(show) q << moveToString(m) << ": " << mv << '\n';

        numPos += mv;

        board.unmakeMove(m, ep, castleRights);
    }
    return numPos;
}
