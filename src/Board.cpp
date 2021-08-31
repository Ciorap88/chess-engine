#include <bits/stdc++.h>

#include "Board.h"
#include "MagicBitboards.h"
#include "TranspositionTable.h"
#include "Search.h"

using namespace std;

unordered_map<U64, char> repetitionMap;

U64 bits[64];
U64 filesBB[8], ranksBB[8], knightAttacksBB[64], kingAttacksBB[64], whitePawnAttacksBB[64], blackPawnAttacksBB[64];
U64 squaresNearWhiteKing[64], squaresNearBlackKing[64];
U64 lightSquaresBB, darkSquaresBB;
U64 castleMask[4];
U64 bishopMasks[64], rookMasks[64];

char castleStartSq[4] = {e1,e1,e8,e8};
char castleEndSq[4] = {g1,c1,g8,c8};

stack<char> castleStk, epStk;

string moveToString(Move m) {
    if(m.from == -1 || m.from == m.to) return "0000";

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

string square(char x) {
    string s;
    s += ('a'+x%8);
    s += ('1'+x/8);
    return s;
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

bool isInBoard(char sq, char dir) {
    char File = (sq & 7);
    char Rank = (sq >> 3);

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

void Init() {
    // bitmasks for every bit from 0 to 63
    for(char i = 0; i < 64; i++)
        bits[i] = (1LL << i);

    // bitboard for checking empty squares between king and rook when castling
    castleMask[0] = (bits[f1] | bits[g1]);
    castleMask[1] = (bits[b1] | bits[c1] | bits[d1]);
    castleMask[2] = (bits[f8] | bits[g8]);
    castleMask[3] = (bits[b8] | bits[c8] | bits[d8]);

    // initialize zobrist numbers in order to make zobrist hash keys
    generateZobristHashNumbers();

    // create file and rank masks
    for(char i = 0; i < 64; i++) {
        char File = (i & 7), Rank = (i >> 3);

        filesBB[File] |= bits[i];
        ranksBB[Rank] |= bits[i];
    }

    // create pawn attacks masks and vectors
    for(char i = 0; i < 64; i++) {
        char File = (i & 7), Rank = (i >> 3);

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
    for(char i = 0; i < 64; i++) {
        char File = (i & 7), Rank = (i >> 3);

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
    for(char i = 0; i < 64; i++) {
        for(auto dir: {NorthEast, NorthWest, SouthEast, SouthWest}) {
            char sq = i;
            while(isInBoard(sq, dir)) {
                bishopMasks[i] |= bits[sq];
                sq += dir;
            }
        }
        bishopMasks[i] ^= bits[i];
    }

    // create rook masks
    for(char i = 0; i < 64; i++) {
        for(auto dir: {East, West, North, South}) {
            char sq = i;
            while(isInBoard(sq, dir)) {
                rookMasks[i] |= bits[sq];
                sq += dir;
            }
        }
        rookMasks[i] ^= bits[i];
    }

    // create king moves masks and vectors
    for(char i = 0; i < 64; i++) {
        kingAttacksBB[i] = (eastOne(bits[i]) | westOne(bits[i]));
        U64 king = (bits[i] | kingAttacksBB[i]);
        kingAttacksBB[i] |= (northOne(king) | southOne(king));
    }

    // squares near king are squares that a king can move to and the squares in front of his 'forward' moves
    for(char i = 0; i < 64; i++) {
        squaresNearWhiteKing[i] = squaresNearBlackKing[i] = (kingAttacksBB[i] | bits[i]);
        if(i+South >= 0) squaresNearBlackKing[i] |= kingAttacksBB[i+South];
        if(i+North < 64) squaresNearWhiteKing[i] |= kingAttacksBB[i+North];
    }

    // create light and dark squares masks
    for(char i = 0; i < 64; i++) {
        if(i%2) lightSquaresBB |= bits[i];
        else darkSquaresBB |= bits[i];
    }

    initMagics();
}

// returns the direction of the move if any, or 0 otherwise
char direction(char from, char to) {
    char fromRank = (from >> 3), fromFile = (from & 7);
    char toRank = (to >> 3), toFile = (to & 7);

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
U64 pawnAttacks (U64 pawns, char color) {
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

void Board::updatePieceInBB(char piece, char color, char sq) {
    if(color == White) this->whitePiecesBB ^= bits[sq];
    else this->blackPiecesBB ^= bits[sq];

    if(piece == Pawn) this->pawnsBB ^= bits[sq];
    if(piece == Knight) this->knightsBB ^= bits[sq];
    if(piece == Bishop) this->bishopsBB ^= bits[sq];
    if(piece == Rook) this->rooksBB ^= bits[sq];
    if(piece == Queen) this->queensBB ^= bits[sq];
}

void Board::movePieceInBB(char piece, char color, char from, char to) {
    this->updatePieceInBB(piece, color, from);
    this->updatePieceInBB(piece, color, to);
}

string Board::getFenFromCurrPos() {
    unordered_map<char, char> pieceSymbols = {{Pawn, 'p'}, {Knight, 'n'},
    {Bishop, 'b'}, {Rook, 'r'}, {Queen, 'q'}, {King, 'k'}};

    string fen;

    char emptySquares = 0;
    for(char Rank = 7; Rank >= 0; Rank--)
        for(char File = 0; File < 8; File++) {
            char color = (this->squares[Rank*8 + File] & (Black | White));
            char piece = (this->squares[Rank*8 + File] ^ color);

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

void Board::loadFenPos(string input) {
    this->blackPiecesBB = this->whitePiecesBB = 0;
    this->pawnsBB = this->knightsBB = 0;
    this->bishopsBB = this->rooksBB = 0;
    this->queensBB = 0;

    // parse the fen string
    string pieces;
    char i = 0;
    for(; input[i] != ' ' && i < input.length(); i++)
        pieces += input[i];
    i++;

    char turn = input[i];
    i += 2;

    string castles;
    for(; input[i] != ' ' && i < input.length(); i++)
        castles += input[i];
    i++;
            
    string epTargetSq;
    for(; input[i] != ' ' && i < input.length(); i++) 
        epTargetSq += input[i];

    unordered_map<char, char> pieceSymbols = {{'p', Pawn}, {'n', Knight},
    {'b', Bishop}, {'r', Rook}, {'q', Queen}, {'k', King}};

    // iterate through squares
    char File = 0, Rank = 7;
    for(char p: pieces) {
        if(p == '/') {
            Rank--;
            File = 0;
        } else {
            // if it is a digit skip the squares
            if(p-'0' >= 1 && p-'0' <= 8) {
                for(char i = 0; i < (p-'0'); i++) {
                    this->squares[Rank*8 + File] = 0;
                    File++;
                }
            } else {
                char color = ((p >= 'A' && p <= 'Z') ? White : Black);
                char type = pieceSymbols[tolower(p)];

                U64 currBit = bits[Rank*8 + File];

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
    this->hashKey = getZobristHashFromCurrPos();
}

static Move pseudoLegalMoves[512];
short Board::GeneratePseudoLegalMoves() {
    short numberOfMoves = 0;

    char color = this->turn;
    char otherColor = (color ^ (Black | White));

    U64 ourPiecesBB = (color == White ? this->whitePiecesBB : this->blackPiecesBB);
    U64 opponentPiecesBB = (color == White ? this->blackPiecesBB : this->whitePiecesBB);
    U64 allPiecesBB = (this->whitePiecesBB | this->blackPiecesBB);

    // -----pawns-----
    U64 ourPawnsBB = (ourPiecesBB & this->pawnsBB);
    char pawnDir = (color == White ? North : South);
    char pawnStartRank = (color == White ? 1 : 6);
    char pawnPromRank = (color == White ? 7 : 0);

    while(ourPawnsBB) {
        char sq = bitscanForward(ourPawnsBB);
        bool isPromoting = (((sq+pawnDir) >> 3) == pawnPromRank);

        // normal moves and promotions
        if((allPiecesBB & bits[sq+pawnDir]) == 0) {

            if((sq >> 3) == pawnStartRank && (allPiecesBB & bits[sq+2*pawnDir]) == 0)
                pseudoLegalMoves[numberOfMoves++] = {sq, (char)(sq+2*pawnDir), 0, 0, 0, 0};

            if(isPromoting) {
                for(char piece: {Knight, Bishop, Rook, Queen})
                    pseudoLegalMoves[numberOfMoves++] = {sq, (char)(sq+pawnDir), 0, 0, 0, piece};
            } else pseudoLegalMoves[numberOfMoves++] = {sq, (char)(sq+pawnDir), 0, 0, 0, 0};

        }

        // captures and capture-promotions
        U64 pawnCapturesBB = (color == White ? whitePawnAttacksBB[sq] : blackPawnAttacksBB[sq]);
        pawnCapturesBB &= opponentPiecesBB;
        while(pawnCapturesBB) {
            char to = bitscanForward(pawnCapturesBB);
            if(isPromoting) {
                for(char piece: {Knight, Bishop, Rook, Queen})
                    pseudoLegalMoves[numberOfMoves++] = {sq, to, this->squares[to], 0, 0, piece};
            } else pseudoLegalMoves[numberOfMoves++] = {sq, to, this->squares[to], 0, 0, 0};

            pawnCapturesBB &= (pawnCapturesBB-1);

        }

        ourPawnsBB &= (ourPawnsBB-1);
    }

    //-----knights-----
    U64 ourKnightsBB = (this->knightsBB & ourPiecesBB);

    while(ourKnightsBB) {
        char sq = bitscanForward(ourKnightsBB);

        U64 knightMoves = (knightAttacksBB[sq] & ~ourPiecesBB);
        while(knightMoves) {
            char to = bitscanForward(knightMoves);
            pseudoLegalMoves[numberOfMoves++] = {sq, to, this->squares[to], 0, 0, 0};
            knightMoves &= (knightMoves-1);
        }

        ourKnightsBB &= (ourKnightsBB-1);
    }

    //-----king-----
    char kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);
    U64 ourKingMoves = (kingAttacksBB[kingSquare] & ~ourPiecesBB);

    while(ourKingMoves) {
        char to = bitscanForward(ourKingMoves);
        pseudoLegalMoves[numberOfMoves++] = {kingSquare, to, this->squares[to], 0, 0, 0};
        ourKingMoves &= (ourKingMoves-1);
    }

    //-----sliding pieces-----
    U64 rooksQueens = (ourPiecesBB & (this->rooksBB | this->queensBB));
    while(rooksQueens) {
        char sq = bitscanForward(rooksQueens);

        U64 rookMoves = (magicRookAttacks(allPiecesBB, sq) & ~ourPiecesBB);
        while(rookMoves) {
            char to = bitscanForward(rookMoves);
            pseudoLegalMoves[numberOfMoves++] = {sq, to, this->squares[to], 0, 0, 0};
            rookMoves &= (rookMoves-1);
        }

        rooksQueens &= (rooksQueens-1);
    }

    U64 bishopsQueens = (ourPiecesBB & (this->bishopsBB | this->queensBB));
    while(bishopsQueens) {
        char sq = bitscanForward(bishopsQueens);

        U64 bishopMoves = (magicBishopAttacks(allPiecesBB, sq) & ~ourPiecesBB);
        while(bishopMoves) {
            char to = bitscanForward(bishopMoves);
            pseudoLegalMoves[numberOfMoves++] = {sq, to, this->squares[to], 0, 0, 0};
            bishopMoves &= (bishopMoves-1);
        }

        bishopsQueens &= (bishopsQueens-1);
    }

    // -----castles-----
    char allowedCastles = (color == White ? 3 : 12);
    for(char i = 0; i < 4; i++)
        if((this->castleRights & bits[i]) && ((allPiecesBB & castleMask[i]) == 0) && (allowedCastles & bits[i]))
            pseudoLegalMoves[numberOfMoves++] = {castleStartSq[i], castleEndSq[i], 0, 0, 1, 0};


    // -----en passant-----
    if(ep != -1) {
        ourPawnsBB = (this->pawnsBB & ourPiecesBB);
        U64 epBB = ((color == White ? blackPawnAttacksBB[ep] : whitePawnAttacksBB[ep]) & ourPawnsBB);
        while(epBB) {
            char sq = bitscanForward(epBB);
            pseudoLegalMoves[numberOfMoves++] = {sq, ep, (char)(Pawn | otherColor), 1, 0, 0};
            epBB &= (epBB-1);
        }
    }
    return numberOfMoves;
}

bool Board::isAttacked(char sq) {
    char color = this->turn;
    char otherColor = (color ^ (Black | White));
    char otherKingSquare = (otherColor == White ? this->whiteKingSquare : this->blackKingSquare);

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
    char color = this->turn;
    char kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);
    return this->isAttacked(kingSquare);
}

U64 Board::attacksTo(char sq) {
    char color = (this->turn ^ (Black | White));

    U64 ourPiecesBB = (color == White ? this->whitePiecesBB : this->blackPiecesBB);
    U64 allPiecesBB = (this->whitePiecesBB | this->blackPiecesBB);
    U64 pawnAtt = (color == Black ? whitePawnAttacksBB[sq] : blackPawnAttacksBB[sq]);
    U64 rooksQueens = (ourPiecesBB & (this->rooksBB | this->queensBB));
    U64 bishopsQueens = (ourPiecesBB & (this->bishopsBB | this->queensBB));
    char kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);

    U64 res = 0;
    res |= (knightAttacksBB[sq] & ourPiecesBB & this->knightsBB);
    res |= (pawnAtt & this->pawnsBB & ourPiecesBB);
    res |= (kingAttacksBB[sq] & bits[kingSquare]);
    res |= (magicBishopAttacks(allPiecesBB, sq) & bishopsQueens);
    res |= (magicRookAttacks(allPiecesBB, sq) & rooksQueens);

    return res;
}

unsigned char Board::GenerateLegalMoves(Move *moves) {
    char color = this->turn;
    char otherColor = (color ^ (Black | White));
    char kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);
    char otherKingSquare = (otherColor == White ? this->whiteKingSquare : this->blackKingSquare);

    U64 allPiecesBB = (this->whitePiecesBB | this->blackPiecesBB);
    U64 opponentPiecesBB = (color == White ? this->blackPiecesBB : this->whitePiecesBB);
    U64 ourPiecesBB = (color == White ? this->whitePiecesBB : this->blackPiecesBB);

    short pseudoNum = this->GeneratePseudoLegalMoves();
    unsigned char num = 0;

    // remove the king so we can correctly find all squares attacked by sliding pieces, where the king can't go
    if(color == White) this->whitePiecesBB ^= bits[kingSquare];
    else this->blackPiecesBB ^= bits[kingSquare];

    U64 checkingPiecesBB = this->attacksTo(kingSquare);
    char checkingPiecesCnt = popcount(checkingPiecesBB);
    char checkingPieceIndex = bitscanForward(checkingPiecesBB);

    // check ray is the path between the king and the sliding piece checking it
    U64 checkRayBB = 0;
    if(checkingPiecesCnt == 1) {
        checkRayBB |= checkingPiecesBB;
        char piece = (this->squares[checkingPieceIndex]^otherColor);
        char dir = direction(checkingPieceIndex, kingSquare);

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
        char sq = bitscanForward(pinners);

        // get pinned pieces by &-ing attacks from the rook square with attacks from the king square, and then with our own pieces
        pinnedBB |= (magicRookAttacks(allPiecesBB, sq) & magicRookAttacks(allPiecesBB, kingSquare) & ourPiecesBB);
        pinners &= (pinners-1); // remove bit
    }

    attacks = magicBishopAttacks(allPiecesBB, kingSquare);
    blockers = (ourPiecesBB & attacks);
    U64 oppBishopsQueens = ((this->bishopsBB | this->queensBB) & opponentPiecesBB);
    pinners = ((attacks ^ magicBishopAttacks((allPiecesBB ^ blockers), kingSquare)) & oppBishopsQueens);
    while(pinners) {
        char sq = bitscanForward(pinners);
        pinnedBB |= (magicBishopAttacks(allPiecesBB, sq) & magicBishopAttacks(allPiecesBB, kingSquare) & ourPiecesBB);
        pinners &= (pinners-1);
    }

    for(int idx = 0; idx < pseudoNum; idx++) {
        Move m = pseudoLegalMoves[idx];

        // we can always move the king to a safe square
        if(m.from == kingSquare) {
            if(this->isAttacked(m.to) == false) moves[num++] = m;
            continue;
        }

        // single check, can capture the attacker or intercept the check only if the moving piece is not pinned
        if(checkingPiecesCnt == 1 && ((pinnedBB & bits[m.from]) == 0)) {

            // capturing the checking pawn by en passant (special case)
            if(m.ep && checkingPieceIndex == m.to + (color == White ? South: North))
                moves[num++] = m;

            // check ray includes interception or capturing the attacker
            else if(checkRayBB & bits[m.to]) {
                moves[num++] = m;
            }
        }

        // no checks, every piece can move if it is not pinned or it moves in the direction of the pin
        if(checkingPiecesCnt == 0) {
            // not pinned
            if((pinnedBB & bits[m.from]) == 0)
                moves[num++] = m;

            // pinned, can only move in the pin direction
            else if(abs(direction(m.from, m.to)) == abs(direction(m.from, kingSquare)))
                moves[num++] = m;
        }
    }

    // en passant are the last added pseudo legal moves
    Move epMoves[2];
    int numEp = 0;
    while(num && moves[num-1].ep) {
        epMoves[numEp++] = moves[--num];
    }
    assert(numEp <= 2);

    // before ep there are the castle moves so we do the same
    Move castleMoves[2];
    int numCastles = 0;
    while(num && moves[num-1].castle) {
        castleMoves[numCastles++] = moves[--num];
    }
    assert(numCastles <= 2);

    // manual check for the legality of castle moves
    for(char idx = 0; idx < numCastles; idx++) {
        char first = min(castleMoves[idx].from, castleMoves[idx].to);
        char second = max(castleMoves[idx].from, castleMoves[idx].to);

        bool ok = true;

        for(char i = first; i <= second; i++)
            if(this->isAttacked(i))
                ok = false;

        if(ok) moves[num++] = castleMoves[idx];
            // moves.push_back(m);
    }

    // ep horizontal pin
    for(char idx = 0; idx < numEp; idx++) {
        char epRank = (epMoves[idx].from >> 3);
        char otherPawnSquare = (epRank << 3) | (epMoves[idx].to & 7);
        U64 rooksQueens = (opponentPiecesBB & (this->rooksBB | this->queensBB) & ranksBB[epRank]);

        // remove the 2 pawns and compute attacks from rooks/queens on king square
        U64 removeWhite = bits[epMoves[idx].from], removeBlack = bits[otherPawnSquare];
        if(color == Black) swap(removeWhite, removeBlack);

        this->whitePiecesBB ^= removeWhite;
        this->blackPiecesBB ^= removeBlack;

        U64 attacks = this->attacksTo(kingSquare);
        if((rooksQueens & attacks) == 0) moves[num++] = epMoves[idx];
            // moves.push_back(enPassant);

        this->whitePiecesBB ^= removeWhite;
        this->blackPiecesBB ^= removeBlack;
    }

    // put the king back
    if(color == White) this->whitePiecesBB ^= bits[kingSquare];
    else this->blackPiecesBB ^= bits[kingSquare];

    return num;
}

void Board::makeMove(Move m) {
    this->updateHashKey(m);

    if(m.from == -1) { // null move
        epStk.push(this->ep);
        this->ep = -1;
        this->turn ^= (Black | White);

        return;
    }
    repetitionMap[this->hashKey]++;

    // push the current castle and ep info in order to retrieve it when we unmake the move
    castleStk.push(this->castleRights);
    epStk.push(this->ep);

    char color = (this->squares[m.from] & (Black | White));
    char piece = (this->squares[m.from] ^ color);
    char otherColor = (color ^ (Black | White));
    char otherPiece = (m.capture ^ otherColor);

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
        char mask = (color == White ? 12 : 3);
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
        char Rank = (m.to >> 3), File = (m.to & 7);
        char rookStartSquare = (Rank << 3) + (File == 6 ? 7 : 0);
        char rookEndSquare = (Rank << 3) + (File == 6 ? 5 : 3);

        this->movePieceInBB(Rook, color, rookStartSquare, rookEndSquare);
        swap(this->squares[rookStartSquare], this->squares[rookEndSquare]);
    }
    // remove the captured pawn if en passant
    if(m.ep) {
        char capturedPawnSquare = m.to+(color == White ? South : North);

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
void Board::unmakeMove(Move m) {
    if(m.from == -1) { // null move
        this->ep = epStk.top();
        epStk.pop();
        this->turn ^= (Black | White);

        this->updateHashKey(m);

        return;
    }
    repetitionMap[this->hashKey]--;

    char color = (this->squares[m.to] & (Black | White));
    char otherColor = (color ^ (Black | White));
    char piece = (this->squares[m.to] ^ color);
    char otherPiece = (m.capture ^ otherColor);

    // retrieve previous castle and ep info
    this->ep = epStk.top();
    this->castleRights = castleStk.top();
    epStk.pop();
    castleStk.pop();

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
        char Rank = (m.to >> 3), File = (m.to & 7);
        char rookStartSquare = (Rank << 3) + (File == 6 ? 7 : 0);
        char rookEndSquare = (Rank << 3) + (File == 6 ? 5 : 3);

        this->movePieceInBB(Rook, color, rookEndSquare, rookStartSquare);
        swap(this->squares[rookStartSquare], this->squares[rookEndSquare]);
    }

    if(m.ep) {
        char capturedPawnSquare = m.to+(color == White ? South : North);

        this->updatePieceInBB(Pawn, otherColor, capturedPawnSquare);
        this->squares[capturedPawnSquare] = (otherColor | Pawn);
        this->squares[m.to] = Empty;
    }

    this->turn ^= (Black | White);

    this->updateHashKey(m);
}

// draw by insufficient material or repetition
bool Board::isDraw() {
    if(repetitionMap[this->hashKey] > 1) return true; // repetition

    if(this->queensBB | this->rooksBB | this->pawnsBB) return false;

    if((this->knightsBB | this->bishopsBB) == 0) return true; // king vs king

    char whiteBishops = popcount(this->bishopsBB | this->whitePiecesBB);
    char blackBishops = popcount(this->bishopsBB | this->blackPiecesBB);
    char whiteKnights = popcount(this->knightsBB | this->whitePiecesBB);
    char blackKnights = popcount(this->knightsBB | this->blackPiecesBB);

    if(whiteKnights + blackKnights + whiteBishops + blackBishops == 1) return true; // king and minor piece vs king

    if(whiteKnights + blackKnights == 0 && whiteBishops == 1 && blackBishops == 1) {
        char lightSquareBishops = popcount(lightSquaresBB & this->bishopsBB);
        if(lightSquareBishops == 0 || lightSquareBishops == 2) return true; // king and bishop vs king and bishop with same color bishops
    }

    return false;
}

Board board;
