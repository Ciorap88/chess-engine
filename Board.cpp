#include <bits/stdc++.h>

#include "Board.h"

using namespace std;

U64 bits[64];
U64 filesBB[8], ranksBB[8], knightAttacksBB[64], kingAttacksBB[64], whitePawnAttacksBB[64], blackPawnAttacksBB[64];
U64 squaresNearWhiteKing[64], squaresNearBlackKing[64];
U64 lightSquaresBB, darkSquaresBB;
U64 castleMask[4];

int castleStartSq[4] = {e1,e1,e8,e8};
int castleEndSq[4] = {g1,c1,g8,c8};

vector<int> promPieces = {Knight, Bishop, Rook, Queen};
vector<int> piecesDirs[8];
vector<int> knightTargetSquares[64];
vector<int> kingMoves[64];
vector<int> whitePawnCaptures[64], blackPawnCaptures[64];

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
   int ans = 0;
   while (bb) {
       ans++;
       bb &= bb-1;
   }
   return ans;
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

vector<unsigned long long> zobristNumbers;

// indices in zobristNumbers vector
const int zBlackTurnIndex = 12*64;
const int zCastleRightsIndex = 12*64+1;
const int zEpFileIndex = 12*64+17;

unsigned long long randomULL() {
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

    // directions in which sliding pieces go
    piecesDirs[Bishop] = {NorthEast, NorthWest, SouthEast, SouthWest};
    piecesDirs[Rook] = {East, West, North, South};
    piecesDirs[Queen] = {NorthEast, NorthWest, SouthEast, SouthWest, East, West, North, South};

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
            whitePawnAttacksBB[i] |= bits[i+7];
            blackPawnAttacksBB[i] |= bits[i-9];

            whitePawnCaptures[i].push_back(i+7);
            blackPawnCaptures[i].push_back(i-9);
        }
        if(File < 7) {
            whitePawnAttacksBB[i] |= bits[i+9];
            blackPawnAttacksBB[i] |= bits[i-7];

            whitePawnCaptures[i].push_back(i+9);
            blackPawnCaptures[i].push_back(i-7);
        }
    }

    // create knight attacks masks and vectors
    for(int i = 0; i < 64; i++) {
        int File = (i & 7), Rank = (i >> 3);

        if(File > 1) {
            if(Rank > 0) {
                knightTargetSquares[i].push_back(i-10);
                knightAttacksBB[i] |= bits[i-10];
            }
            if(Rank < 7) {
                knightTargetSquares[i].push_back(i+6);
                knightAttacksBB[i] |= bits[i+6];
            }
        }
        if(File < 6) {
            if(Rank > 0) {
                knightTargetSquares[i].push_back(i-6);
                knightAttacksBB[i] |= bits[i-6];
            }
            if(Rank < 7) {
                knightTargetSquares[i].push_back(i+10);
                knightAttacksBB[i] |= bits[i+10];
            }
        }
        if(Rank > 1) {
            if(File > 0) {
                knightTargetSquares[i].push_back(i-17);
                knightAttacksBB[i] |= bits[i-17];
            }
            if(File < 7) {
                knightTargetSquares[i].push_back(i-15);
                knightAttacksBB[i] |= bits[i-15];
            }
        }
        if(Rank < 6) {
            if(File > 0) {
                knightTargetSquares[i].push_back(i+15);
                knightAttacksBB[i] |= bits[i+15];
            }
            if(File < 7) {
                knightTargetSquares[i].push_back(i+17);
                knightAttacksBB[i] |= bits[i+17];
            }
        }
    }

    // create king moves masks and vectors
    for(int i = 0; i < 64; i++) {
        kingAttacksBB[i] = (eastOne(bits[i]) | westOne(bits[i]));
        U64 king = (bits[i] | kingAttacksBB[i]);
        kingAttacksBB[i] |= (northOne(king) | southOne(king));

        for(int j = 0; j < 64; j++)
            if(kingAttacksBB[i] & bits[j])
                kingMoves[i].push_back(j);

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

    int pawnDir = (color == White ? North : South);
    int pawnStartRank = (color == White ? 1 : 6);
    int pawnPromRank = (color == White ? 7 : 0);

    vector<Move> moves;

    for(int i = 0; i < 64; i++) {
        if(this->squares[i] == Empty || (this->squares[i] & color) == 0) continue;
        int piece = (this->squares[i]) - color;
        int Rank = (i >> 3);
        int File = (i & 7);

        // moves for sliding pieces
        if(piece == Bishop || piece == Queen || piece == Rook)  {
            for(auto dir: piecesDirs[piece]) {
                int curSquare = i;

                while(isInBoard(curSquare, dir) && ((this->squares[curSquare+dir] & color) == 0)) {
                    curSquare += dir;
                    moves.push_back({i, curSquare, this->squares[curSquare], 0, 0, 0});
                    if(this->squares[curSquare] != Empty) break;
                }
            }
        }

        // moves, captures and promotions for pawns
        if(piece == Pawn) {
            bool isPromoting = (((i+pawnDir) >> 3) == pawnPromRank);
            vector<int> pawnCaptures = (color == White ? whitePawnCaptures[i] : blackPawnCaptures[i]);

            // normal moves and promotions
            if((allPiecesBB & bits[i+pawnDir]) == 0) {

                if(Rank == pawnStartRank && (allPiecesBB & bits[i+2*pawnDir]) == 0)
                    moves.push_back({i, i+2*pawnDir, 0, 0, 0, 0}); // 2 square move

                if(isPromoting) {
                    for(int pc: promPieces)
                        moves.push_back({i, i+pawnDir, 0, 0, 0 , pc});
                } else {
                    moves.push_back({i, i+pawnDir, 0, 0, 0 , 0});
                }
            }

            // captures and capture-promotions
            for(int sq : pawnCaptures) {
                if(bits[sq] & opponentPiecesBB) {
                    if(isPromoting) {
                        for(auto pc: promPieces)
                            moves.push_back({i, sq, this->squares[sq], 0, 0, pc});
                    } else {
                        moves.push_back({i, sq, this->squares[sq], 0, 0, 0});
                    }
                }
            }
        }

        // moves for knights
        if(piece == Knight) {
            for(auto sq: knightTargetSquares[i]) {
                if((ourPiecesBB & bits[sq]) == 0) {
                    moves.push_back({i, sq, this->squares[sq], 0, 0, 0});
                }
            }
        }

        // moves for kings
        if(piece == King) {
            for(int sq: kingMoves[i]) {
                if((ourPiecesBB & bits[sq]) == 0)
                    moves.push_back({i, sq, this->squares[sq], 0, 0, 0});
            }
        }
    }

    // castles
    for(int i = 0; i < 4; i++)
        if((this->castleRights & bits[i]) && ((allPiecesBB & castleMask[i]) == 0))
             moves.push_back({castleStartSq[i], castleEndSq[i], 0, 0, 1, 0});

    // en passant
    if(ep != -1) {
        vector<int> dirs;
        if(color == White) dirs = {SouthWest, SouthEast};
        else dirs = {NorthWest, NorthEast};

        for(int dir: dirs)
            if(isInBoard(ep, dir) && this->squares[ep+dir] == (Pawn | color))
                moves.push_back({ep+dir, ep, (Pawn | otherColor), 1, 0, 0});
    }

    return moves;
}

// returns true if the current player to move is in check
bool Board::isInCheck() {
    int color = this->turn;
    int otherColor = (color ^ (Black | White));
    int kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);

    U64 opponentPiecesBB = (color == White ? this->blackPiecesBB : this->whitePiecesBB);

    // knight checks
    if(knightAttacksBB[kingSquare] & opponentPiecesBB & this->knightsBB)
        return true;

    // pawn checks
    if(bits[kingSquare] & pawnAttacks((opponentPiecesBB & this->pawnsBB), otherColor))
        return true;

    // sliding piece checks
    vector<int> slidingPieces = {Bishop, Rook, Queen};
    for(int piece: slidingPieces) {
        for(int dir: piecesDirs[piece]) {
            for(int i = kingSquare; ; i += dir) {
                if(this->squares[i] == (otherColor | piece)) return true;
                if((i != kingSquare && this->squares[i] != Empty) || !isInBoard(i, dir)) break;
            }
        }
    }

    return false;
}

vector<Move> Board::GenerateLegalMoves() {
    int color = this->turn;
    int otherColor = (color ^ (Black | White));

    int kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);
    int otherKingSquare = (otherColor == White ? this->whiteKingSquare : this->blackKingSquare);

    vector<Move> potentialMoves = this->GeneratePseudoLegalMoves();

    // remove the king before calculating attacked squares by opponent's sliding pieces
    this->squares[kingSquare] = Empty;

    vector<int> checkingPieces;
    U64 attackedSquaresBB = 0;

    // attacked squares for sliding pieces
    for(int slidingPiece: {Rook, Bishop, Queen}) {
        for(int i = 0; i < 64; i++) {
            if(this->squares[i] == (slidingPiece | otherColor)) {
                for(int dir : piecesDirs[slidingPiece]) {
                    for(int j = i; ; j += dir) {
                        if(j != i) attackedSquaresBB |= bits[j];
                        if(j == kingSquare) {
                            checkingPieces.push_back(i);
                        }
                        if((this->squares[j] != Empty && j != i) || !isInBoard(j, dir)) break;
                    }
                }
            }
        }
    }

    this->squares[kingSquare] = (King | color);

    // squares attacked by knights
    for(int i = 0; i < 64; i++) {
        if(this->squares[i] == (Knight | otherColor)) {
            attackedSquaresBB |= knightAttacksBB[i];
            if(knightAttacksBB[i] & bits[kingSquare])
                checkingPieces.push_back(i);
        }
    }

    // squares attacked by the opponent king
    attackedSquaresBB |= kingAttacksBB[otherKingSquare];

    // squares attacked by pawns
    for(int i = 0; i < 64; i++) {
        if(this->squares[i] == (Pawn | otherColor)) {
            U64 attacks = (otherColor == Black ? blackPawnAttacksBB[i] : whitePawnAttacksBB[i]);

            attackedSquaresBB |= attacks;
            if(attacks & bits[kingSquare])
                checkingPieces.push_back(i);
        }
    }

    // check ray is the path between the king and the sliding piece checking it
    U64 checkRayBB = 0;
    if(checkingPieces.size() == 1) {
        checkRayBB |= bits[checkingPieces[0]];

        int piece = (this->squares[checkingPieces[0]]^otherColor);

        if(piece == Bishop || piece == Queen || piece == Rook) {
            int rayDir = direction(checkingPieces[0], kingSquare);
            for(int i = checkingPieces[0]; i != kingSquare; i += rayDir)
                checkRayBB |= bits[i];
        }
    }
    U64 pinnedBB = 0;
    vector<int> pinDirection(64, 0);

    // finding the absolute pins
    for(int pinner: {Rook, Queen, Bishop}) {
        for(int dir: piecesDirs[pinner]) {
            U64 kingRayBB = 0;
            for(int i = kingSquare; ; i += dir) {
                kingRayBB |= bits[i];
                if((i != kingSquare && this->squares[i] != Empty) || !isInBoard(i, dir))
                    break;
            }

            for(int i = kingSquare; ; i += dir) {
                if(this->squares[i] == (pinner | otherColor)) {
                    for(int j = i; ; j -= dir) {

                        // the intersection of the king ray and the other piece ray in the opposite direction
                        if(kingRayBB & bits[j]) {
                            pinnedBB |= bits[j];
                            pinDirection[j] = dir;
                        }
                        if(!isInBoard(j, -dir) || (this->squares[j] != Empty && j != i))
                            break;
                    }
                    break;
                }
                if(!isInBoard(i, dir)) break;
            }
        }
    }

    vector<Move> epMoves;
    vector<Move> moves;

    for(Move m: potentialMoves) {
        // we can always move the king to a safe square
        if(m.from == kingSquare) {
            if((attackedSquaresBB & bits[m.to]) == 0) moves.push_back(m);
            continue;
        }

        // single check, can capture the attacker or intercept the check only if the moving piece is not pinned
        if(checkingPieces.size() == 1 && ((pinnedBB & bits[m.from]) == 0)) {

            // capturing the checking pawn by en passant (special case)
            if(m.ep && checkingPieces[0] == m.to + (color == White ? South: North))
                moves.push_back(m);

            // check ray includes interception or capturing the attacker
            else if(checkRayBB & bits[m.to])
                moves.push_back(m);
        }

        // no checks, every piece can move if it is not pinned or it moves in the direction of the pin
        if(checkingPieces.size() == 0) {
            // not pinned
            if((pinnedBB & bits[m.from]) == 0)
                moves.push_back(m);

            // pinned, can only move in the pin direction
            else if(abs(direction(m.from, m.to)) == abs(pinDirection[m.from]))
                moves.push_back(m);
        }
    }

    // en passant are the last added pseudo legal moves so we know they are at the back of the vector if they exist
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
            if(attackedSquaresBB & bits[i])
                ok = false;

        if(ok) moves.push_back(m);
    }

    // ep horizontal pin
    for(Move enPassant: epMoves) {
        int dir = direction(enPassant.from, enPassant.to);

        // go in the ray direction from the 2 pawns and see if we find a king and an opposite colored queen/rook
        int rayDir = ((dir == NorthWest || dir == SouthWest) ? East : West);
        pair<int, int> pieces = {0,0};

        for(int i = enPassant.from; ; i += rayDir) {
            if(this->squares[i] != Empty && i != enPassant.from) {
                pieces.first = this->squares[i];
                break;
            }
            if(!isInBoard(i, rayDir)) break;
        }
        for(int i = enPassant.from-rayDir; ;i -= rayDir) {
            if(this->squares[i] != Empty && i != enPassant.from-rayDir) {
                pieces.second = this->squares[i];
                break;
            }
            if(!isInBoard(i, -rayDir)) break;
        }
        if(pieces.second == (color | King))
            swap(pieces.first, pieces.second);
        if(!(pieces.first == (color | King) && (pieces.second == (otherColor | Queen) || pieces.second == (otherColor | Rook)))) {
            moves.push_back(enPassant);
        }
    }
    return moves;
}

void Board::makeMove(Move m) {
    int color = (this->squares[m.from] & (Black | White));
    int piece = (this->squares[m.from] ^ color);

    int otherColor = (color ^ (Black | White));
    int otherPiece = (m.capture ^ otherColor);

    // update bitboards
    this->updatePieceInBB(piece, color, m.from);
    if(!m.ep && m.capture) this->updatePieceInBB(otherPiece, otherColor, m.to);
    if(!m.prom) this->updatePieceInBB(piece, color, m.to);


    // update zobrist hash key
    this->hashKey ^= zobristNumbers[zPieceSquareIndex(piece, color, m.from)];
    if(!m.prom) this->hashKey ^= zobristNumbers[zPieceSquareIndex(piece, color, m.to)];
    if(m.capture && !m.ep) this->hashKey ^= zobristNumbers[zPieceSquareIndex(otherPiece, otherColor, m.to)];


    this->squares[m.to] = this->squares[m.from];
    this->squares[m.from] = Empty;

    // promote pawn
    if(m.prom) {
        this->updatePieceInBB(m.prom, color, m.to);
        this->squares[m.to] = (m.prom | color);
        this->hashKey ^= zobristNumbers[zPieceSquareIndex(m.prom, color, m.to)];
    }

    // remove castle rights
    this->hashKey ^= zobristNumbers[zCastleRightsIndex + this->castleRights];

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

    // add the new castle rights
    this->hashKey ^= zobristNumbers[zCastleRightsIndex + this->castleRights];


    // move the rook if castle
    if(m.castle) {
        if(m.to == c1) {
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, White, a1)];
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, White, d1)];

            this->movePieceInBB(Rook, color, a1, d1);

            swap(this->squares[a1], this->squares[d1]);

        } else if(m.to == g1) {
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, White, f1)];
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, White, h1)];

            this->movePieceInBB(Rook, color, f1, h1);

            swap(this->squares[h1], this->squares[f1]);

        } else if(m.to == g8) {
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, Black, f8)];
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, Black, h8)];

            this->movePieceInBB(Rook, color, f8, h8);

            swap(this->squares[f8], this->squares[h8]);

        } else {
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, Black, a8)];
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, Black, d8)];

            this->movePieceInBB(Rook, color, a8, d8);

            swap(this->squares[a8], this->squares[d8]);
        }
    }
    // remove the captured pawn if en passant
    if(m.ep) {
        int capturedPawnSquare = m.to+(color == White ? South : North);

        this->updatePieceInBB(Pawn, otherColor, capturedPawnSquare);
        this->squares[capturedPawnSquare] = Empty;

        this->hashKey ^= zobristNumbers[zPieceSquareIndex(Pawn, otherColor, capturedPawnSquare)];
    }

    // update en passant target square
    if(this->ep != -1) this->hashKey ^= zobristNumbers[zEpFileIndex + (this->ep % 8)];

    this->ep = -1;
    if(piece == Pawn && abs(m.from-m.to) == 16)
        this->ep = m.to+(color == White ? -8 : 8);

    if(this->ep != -1) this->hashKey ^= zobristNumbers[zEpFileIndex + (this->ep % 8)];

    // switch turn
    this->turn ^= (Black | White);
    this->hashKey ^= zobristNumbers[zBlackTurnIndex];
}

// basically the inverse of makeMove but we need to memorize the castling right and ep square before the move
void Board::unmakeMove(Move m, int ep, int castleRights) {
    int color = (this->squares[m.to] & (Black | White));
    int otherColor = (color ^ (Black | White));
    int piece = (this->squares[m.to] ^ color);
    int otherPiece = (m.capture ^ otherColor);

    if(this->ep != -1)
        this->hashKey ^= zobristNumbers[zEpFileIndex + (this->ep % 8)];

    if(ep != -1)
        this->hashKey ^= zobristNumbers[zEpFileIndex + (ep % 8)];

    this->hashKey ^= zobristNumbers[zCastleRightsIndex + this->castleRights];
    this->hashKey ^= zobristNumbers[zCastleRightsIndex + castleRights];

    this->ep = ep;
    this->castleRights = castleRights;

    if(piece == King) {
        if(color == White) this->whiteKingSquare = m.from;
        if(color == Black) this->blackKingSquare = m.from;
    }

    this->updatePieceInBB(piece, color, m.to);

    if(m.prom) {
        this->squares[m.to] = (Pawn | color);
        this->hashKey ^= zobristNumbers[zPieceSquareIndex(Pawn, color, m.from)];
    }

    this->updatePieceInBB((this->squares[m.to] ^ color), color, m.from);
    if(m.capture && !m.ep) this->updatePieceInBB(otherPiece, otherColor, m.to);

    this->hashKey ^= zobristNumbers[zPieceSquareIndex(piece, color, m.to)];
    if(m.capture && !m.ep) this->hashKey ^= zobristNumbers[zPieceSquareIndex(otherPiece, otherColor, m.to)];
    if(!m.prom) this->hashKey ^= zobristNumbers[zPieceSquareIndex(piece, color, m.from)];


    this->squares[m.from] = this->squares[m.to];
    this->squares[m.to] = m.capture;

    if(m.castle) {
        if(m.to == c1) {
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, White, a1)];
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, White, d1)];

            this->movePieceInBB(Rook, color, a1, d1);

            swap(this->squares[a1], this->squares[d1]);

        } else if(m.to == g1) {
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, White, h1)];
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, White, f1)];

            this->movePieceInBB(Rook, color, f1, h1);

            swap(this->squares[f1], this->squares[h1]);

        } else if(m.to == g8) {
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, Black, f8)];
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, Black, h8)];

            this->movePieceInBB(Rook, color, f8, h8);

            swap(this->squares[f8], this->squares[h8]);

        } else {
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, Black, a8)];
            this->hashKey ^= zobristNumbers[zPieceSquareIndex(Rook, Black, d8)];

            this->movePieceInBB(Rook, color, a8, d8);

            swap(this->squares[a8], this->squares[d8]);
        }
    }

    if(m.ep) {
        int capturedPawnSquare = m.to+(color == White ? South : North);

        this->updatePieceInBB(Pawn, otherColor, capturedPawnSquare);
        this->hashKey ^= zobristNumbers[zPieceSquareIndex(Pawn, otherColor, capturedPawnSquare)];

        this->squares[capturedPawnSquare] = (otherColor | Pawn);
        this->squares[m.to] = Empty;
    }

    this->turn ^= (Black | White);
    this->hashKey ^= zobristNumbers[zBlackTurnIndex];
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
