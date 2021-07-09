#include <bits/stdc++.h>

#include "Board.h"

using namespace std;

U64 bits[64];
U64 filesBB[8], ranksBB[8], knightAttacksBB[64], kingAttacksBB[64];
U64 squaresNearWhiteKing[64], squaresNearBlackKing[64];
U64 lightSquaresBB, darkSquaresBB;

const string startingPos = "rnbqkbnr/ppppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
vector<int> piecesDirs[8];
vector<int> knightTargetSquares[64];

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
    return ((bb >> 1) & (~filesBB[0]));
}

U64 westOne(U64 bb) {
    return ((bb << 1) & (~filesBB[7]));
}

U64 northOne(U64 bb) {
    return ((bb << 8) & (~ranksBB[0]));
}
U64 southOne(U64 bb) {
    return ((bb >> 8) & (~ranksBB[7]));
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

void Init() {
    for(int i = 0; i < 64; i++)
        bits[i] = (1LL << i);

    piecesDirs[Bishop] = {NorthEast, NorthWest, SouthEast, SouthWest};
    piecesDirs[Rook] = {East, West, North, South};
    piecesDirs[King] = piecesDirs[Queen] = {NorthEast, NorthWest, SouthEast, SouthWest, East, West, North, South};

    generateZobristHashNumbers();

    for(int i = 0; i < 64; i++) {
        int File = i%8, Rank = i/8;

        filesBB[File] |= bits[i];
        ranksBB[Rank] |= bits[i];

        kingAttacksBB[i] = (eastOne(bits[i]) | westOne(bits[i]));
        U64 king = (bits[i] | kingAttacksBB[i]);
        kingAttacksBB[i] |= (northOne(king) | southOne(king));

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

    for(int i = 0; i < 64; i++) {
        squaresNearWhiteKing[i] = squaresNearBlackKing[i] = (kingAttacksBB[i] | bits[i]);
        if(i+South >= 0) squaresNearBlackKing[i] |= kingAttacksBB[i+South];
        if(i+North < 64) squaresNearWhiteKing[i] |= kingAttacksBB[i+North];
    }

    for(int i = 0; i < 64; i++) {
        if(i%2) lightSquaresBB |= bits[i];
        else darkSquaresBB |= bits[i];
    }
}

// returns the direction of the move if any, or 0
int direction(int from, int to) {
    int fromRank = from/8, fromFile = from%8;
    int toRank = to/8, toFile = to%8;
    if(fromRank == toRank) {
        if(to > from) return East;
        else return West;
    }
    if(fromFile == toFile) {
        if(to > from) return North;
        else return South;
    }
    if(fromRank-toRank == fromFile-toFile) {
        if(to > from) return NorthEast;
        else return SouthWest;
    }
    if(fromRank-toRank == toFile-fromFile) {
        if(to > from) return NorthWest;
        else return SouthEast;
    }
    return 0;
}

// manhattan distance
int dist(int sq1, int sq2) {
    int Rank1 = sq1/8;
    int Rank2 = sq2/8;
    int File1 = sq1%8;
    int File2 = sq2%8;

    return abs(Rank1-Rank2) + abs(File1-File2);
}

bool isInBoard(int sq, int dir) {
    int File = sq%8;
    int Rank = sq/8;

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

    return fen;
}

// xor zobrist numbers corresponding to all features of the current position
void Board::initZobristHashFromCurrPos() {
    this->zobristHash = 0;
    for(int i = 0; i < 64; i++) {
        int color = (this->squares[i] & (Black | White));
        int piece = (this->squares[i] ^ color);

        this->zobristHash ^= zobristNumbers[zPieceSquareIndex(piece, color, i)];
    }

    this->zobristHash ^= zobristNumbers[zCastleRightsIndex + this->castleRights];
    if(this->turn == Black) this->zobristHash ^= zobristNumbers[zBlackTurnIndex];

    int epFile = this->ep % 8;
    this->zobristHash ^= zobristNumbers[zEpFileIndex + epFile];
}

void Board::LoadFenPos(string fen) {
    unordered_map<char, int> pieceSymbols = {{'p', Pawn}, {'n', Knight},
    {'b', Bishop}, {'r', Rook}, {'q', Queen}, {'k', King}};

    // index of character after the first space in the fen string
    int index = 0;

    int File = 0, Rank = 7;
    for(char symbol: fen) {
        index++;
        if(symbol == ' ') break;
        if(symbol == '/') {
            Rank--;
            File = 0;
        } else {
            // if it is a digit skip the squares
            if(symbol-'0' >= 1 && symbol-'0' <= 8) {
                for(int i = 0; i < (symbol-'0'); i++) {
                    this->squares[Rank*8 + File] = 0;
                    File++;
                }
            } else {
                int color = ((symbol >= 'A' && symbol <= 'Z') ? White : Black);
                int type = pieceSymbols[tolower(symbol)];

                int currBit = bits[Rank*8 + File];

                this->addPieceInBB(type, color, Rank*8 + File);
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
    this->turn = (fen[index] == 'w' ? White : Black);

    // get castling rights
    index += 2;
    this->castleRights = 0;

    while(fen[index] != ' '){
        if(fen[index] == 'K') this->castleRights |= bits[0];
        if(fen[index] == 'Q') this->castleRights |= bits[1];
        if(fen[index] == 'k') this->castleRights |= bits[2];
        if(fen[index] == 'q') this->castleRights |= bits[3];
        index++;
    }

    // get en passant target square
    index++;
    this->ep = (fen[index] == '-' ? -1 : (fen[index]-'a' + 8*(fen[index+1]-'1')));

    initZobristHashFromCurrPos();
}

vector<Move> Board::GeneratePseudoLegalMoves() {
    int color = this->turn;
    int otherColor = (color ^ (Black | White));

    vector<Move> moves;
    for(int i = 0; i < 64; i++) {
        if(this->squares[i] == Empty || this->squares[i] & color == 0) continue;
        int piece = (this->squares[i]) - color;
        int Rank = i/8;
        int File = i%8;

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
            int dir = (color == White ? North : South);
            int startRank = (color == White ? 1 : 6);
            int promRank = (color == White ? 7 : 0);
            vector<int> promPieces = {Knight, Bishop, Rook, Queen};

            // 2 square move at the beginning
            if(Rank == startRank && this->squares[i+dir] == Empty && this->squares[i+2*dir] == Empty) {
                moves.push_back({i, i+2*dir, 0, 0, 0, 0});
            }

            // normal moves and promotions
            if(Rank != promRank && this->squares[i+dir] == Empty) {
                if((i+dir)/8 == promRank) {
                    for(auto pc: promPieces)
                        moves.push_back({i, i+dir, 0, 0, 0 , pc});
                } else {
                    moves.push_back({i, i+dir, 0, 0, 0 , 0});
                }
            }

            // captures and capture-promotions
            for(int side: {East, West}) {
                if(Rank != promRank && isInBoard(i+dir, side) && this->squares[i+dir+side] != Empty && (this->squares[i+dir+side] & color) == 0) {
                    if((i+dir)/8 == promRank) {
                        for(auto pc: promPieces)
                            moves.push_back({i, i+dir+side, this->squares[i+dir+side], 0, 0, pc});
                    } else {
                        moves.push_back({i, i+dir+side, this->squares[i+dir+side], 0, 0, 0});
                    }
                }
            }
        }

        // moves for knights
        if(piece == Knight) {
            for(auto sq: knightTargetSquares[i]) {
                if((this->squares[sq] & color) == 0) {
                    moves.push_back({i, sq, this->squares[sq], 0, 0, 0});
                }
            }
        }

        // moves for kings
        if(piece == King) {
            for(int dir: piecesDirs[King]) {
                if(isInBoard(i, dir) && (this->squares[i+dir] & color) == 0) {
                    moves.push_back({i, i+dir, this->squares[i+dir], 0, 0, 0});
                }
            }
        }
    }

    // castles
    if((this->castleRights & bits[0]) && this->squares[f1] == Empty && this->squares[g1] == Empty)
        moves.push_back({e1, g1, 0, 0, 1, 0});

    if((this->castleRights & bits[1]) && this->squares[b1] == Empty && this->squares[c1] == Empty && this->squares[d1] == Empty)
        moves.push_back({e1, c1, 0, 0, 1, 0});

    if((this->castleRights & bits[2]) && this->squares[f8] == Empty && this->squares[g8] == Empty)
        moves.push_back({e8, g8, 0, 0, 1, 0});

    if((this->castleRights & bits[3]) && this->squares[b8] == Empty && this->squares[c8] == Empty && this->squares[d8] == Empty)
        moves.push_back({e8, c8, 0, 0, 1, 0});

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

    // knight checks
    for(auto sq: knightTargetSquares[kingSquare])
        if(this->squares[sq] == (otherColor | Knight))
            return true;

    // pawn checks
    vector<int> pawnDirs;
    if(color == White) pawnDirs = {NorthWest, NorthEast};
    if(color == Black) pawnDirs = {SouthWest, SouthEast};

    for(int dir: pawnDirs)
        if(isInBoard(kingSquare, dir) && this->squares[kingSquare+dir] == (Pawn | otherColor))
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

    vector<int> attackedSquares(64, 0), checkingPieces;

    // attacked squares for sliding pieces
    for(int slidingPiece: {Rook, Bishop, Queen}) {
        for(int i = 0; i < 64; i++) {
            if(this->squares[i] == (slidingPiece | otherColor)) {
                for(int dir : piecesDirs[slidingPiece]) {
                    for(int j = i; ; j += dir) {
                        attackedSquares[j] ++;
                        if(j == kingSquare) {
                            checkingPieces.push_back(i);
                        }
                        if((this->squares[j] != Empty && j != i) || !isInBoard(j, dir)) break;
                    }
                    attackedSquares[i]--;
                }
            }
        }
    }

    this->squares[kingSquare] = (King | color);

    // squares attacked by knights
    for(int i = 0; i < 64; i++) {
        if(this->squares[i] == (Knight | otherColor))
        for(auto j: knightTargetSquares[i]) {
            attackedSquares[j] ++;
            if(j == kingSquare) {
                checkingPieces.push_back(i);
            }
        }
    }

    // squares attacked by the opponent king
    for(int dir: piecesDirs[King]) {
        if(isInBoard(otherKingSquare, dir)) {
            attackedSquares[otherKingSquare+dir]++;
        }
    }

    // squares attacked by pawns
    for(int i = 0; i < 64; i++) {
        if(this->squares[i] == (Pawn | otherColor)) {
            vector<int> attackDirs;
            if(otherColor == Black) attackDirs = {SouthEast, SouthWest};
            else attackDirs = {NorthEast, NorthWest};

            for(int dir: attackDirs) {
                if(isInBoard(i, dir)) {
                    attackedSquares[i+dir]++;
                    if(i+dir == kingSquare) checkingPieces.push_back(i);
                }
            }
        }
    }

    // check ray is the path between the king and the sliding piece checking it
    set<int> checkRay;
    if(checkingPieces.size() == 1) {
        checkRay.insert(checkingPieces[0]);

        int piece = (this->squares[checkingPieces[0]]^otherColor);

        if(piece == Bishop || piece == Queen || piece == Rook) {
            int rayDir = direction(checkingPieces[0], kingSquare);
            for(int i = checkingPieces[0]; i != kingSquare; i += rayDir) {
                checkRay.insert(i);
            }
        }
    }
    set<int> pinned;
    vector<int> pinDirection(64, 0);

    // finding the absolute pins
    for(int pinner: {Rook, Queen, Bishop}) {
        for(int dir: piecesDirs[pinner]) {
            set<int> kingRay;
            for(int i = kingSquare; ; i += dir) {
                kingRay.insert(i);
                if((i != kingSquare && this->squares[i] != Empty) || !isInBoard(i, dir))
                    break;
            }

            for(int i = kingSquare; ; i += dir) {
                if(this->squares[i] == (pinner | otherColor)) {
                    for(int j = i; ; j -= dir) {

                        // the intersection of the king ray and the other piece ray in the opposite direction
                        if(kingRay.count(j)) {
                            pinned.insert(j);
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
        if(m.from == kingSquare) {
            if(attackedSquares[m.to] == 0) moves.push_back(m);

        // if in double check, we can only move the king
        } else if(checkingPieces.size() > 1) {
            continue;

        // if in single check, we can also intercept the check or capture the checking piece
        } else if(checkingPieces.size() == 1) {
            if(((m.ep && checkingPieces[0] == m.to + (color == White ? South : North)) || checkRay.count(m.to)) && !pinned.count(m.from))
                moves.push_back(m);

        // pinned pieces can only move in the direction of the pin
        } else if(pinned.count(m.from)) {
            if(abs(direction(m.from, m.to)) == abs(pinDirection[m.from]))
                moves.push_back(m);
        } else {
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
            if(attackedSquares[i])
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

void Board::deletePieceInBB(int piece, int color, int sq) {

    if(color == White) this->whitePiecesBB ^= bits[sq];
    else this->blackPiecesBB ^= bits[sq];

    if(piece == Pawn) this->pawnsBB ^= bits[sq];
    if(piece == Knight) this->knightsBB ^= bits[sq];
    if(piece == Bishop) this->bishopsBB ^= bits[sq];
    if(piece == Rook) this->rooksBB ^= bits[sq];
    if(piece == Queen) this->queensBB ^= bits[sq];
}

void Board::addPieceInBB(int piece, int color, int sq) {

    if(color == White) this->whitePiecesBB |= bits[sq];
    else this->blackPiecesBB |= bits[sq];

    if(piece == Pawn) this->pawnsBB |= bits[sq];
    if(piece == Knight) this->knightsBB |= bits[sq];
    if(piece == Bishop) this->bishopsBB |= bits[sq];
    if(piece == Rook) this->rooksBB |= bits[sq];
    if(piece == Queen) this->queensBB |= bits[sq];
}

void Board::makeMove(Move m) {
    int color = (this->squares[m.from] & (Black | White));
    int piece = (this->squares[m.from] ^ color);

    int otherColor = (color ^ (Black | White));
    int otherPiece = (m.capture ^ otherColor);

    // update bitboards
    this->deletePieceInBB(piece, color, m.from);
    if(!m.ep && m.capture) this->deletePieceInBB(otherPiece, otherColor, m.to);
    if(!m.prom) this->addPieceInBB(piece, color, m.to);


    // update zobrist hash
    this->zobristHash ^= zobristNumbers[zPieceSquareIndex(piece, color, m.from)];
    if(!m.prom) this->zobristHash ^= zobristNumbers[zPieceSquareIndex(piece, color, m.to)];
    if(m.capture && !m.ep) this->zobristHash ^= zobristNumbers[zPieceSquareIndex(otherPiece, otherColor, m.to)];


    this->squares[m.to] = this->squares[m.from];
    this->squares[m.from] = Empty;

    // promote pawn
    if(m.prom) {
        this->addPieceInBB(m.prom, color, m.to);
        this->squares[m.to] = (m.prom | color);
        this->zobristHash ^= zobristNumbers[zPieceSquareIndex(m.prom, color, m.to)];
    }

    // remove castle rights
    this->zobristHash ^= zobristNumbers[zCastleRightsIndex + this->castleRights];

    if(piece == King) {

        // bit mask for removing castle rights
        int mask;

        if(color == White) {
            mask = 12;
            this->whiteKingSquare = m.to;
        }
        if(color == Black) {
            mask = 3;
            this->blackKingSquare = m.to;
        }

        this->castleRights &= mask;
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
    this->zobristHash ^= zobristNumbers[zCastleRightsIndex + this->castleRights];


    // move the rook if castle
    if(m.castle) {
        if(m.to == c1) {
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, White, a1)];
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, White, d1)];
            this->addPieceInBB(Rook, color, d1);
            this->deletePieceInBB(Rook, color, a1);

            swap(this->squares[a1], this->squares[d1]);
        } else if(m.to == g1) {
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, White, f1)];
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, White, h1)];
            this->addPieceInBB(Rook, color, f1);
            this->deletePieceInBB(Rook, color, h1);

            swap(this->squares[h1], this->squares[f1]);
        } else if(m.to == g8) {
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, Black, f8)];
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, Black, h8)];
            this->addPieceInBB(Rook, color, f8);
            this->deletePieceInBB(Rook, color, h8);

            swap(this->squares[f8], this->squares[h8]);
        } else {
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, Black, a8)];
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, Black, d8)];
            this->addPieceInBB(Rook, color, d8);
            this->deletePieceInBB(Rook, color, a8);

            swap(this->squares[a8], this->squares[d8]);
        }
    }
    // remove the captured pawn if en passant
    if(m.ep) {
        int capturedPawnSquare = m.to+(color == White ? South : North);

        this->deletePieceInBB(Pawn, otherColor, capturedPawnSquare);
        this->squares[capturedPawnSquare] = Empty;

        this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Pawn, otherColor, capturedPawnSquare)];
    }

    // update en passant target square
    if(this->ep != -1) this->zobristHash ^= zobristNumbers[zEpFileIndex + (this->ep % 8)];

    this->ep = -1;
    if(piece == Pawn && abs(m.from-m.to) == 16)
        this->ep = m.to+(color == White ? -8 : 8);

    if(this->ep != -1) this->zobristHash ^= zobristNumbers[zEpFileIndex + (this->ep % 8)];

    // switch turn
    this->turn ^= (Black | White);
    this->zobristHash ^= zobristNumbers[zBlackTurnIndex];
}

// basically the inverse of makeMove but we need to memorize the castling right and ep square before the move
void Board::unmakeMove(Move m, int ep, int castleRights) {
    int color = (this->squares[m.to] & (Black | White));
    int otherColor = (color ^ (Black | White));
    int piece = (this->squares[m.to] ^ color);
    int otherPiece = (m.capture ^ otherColor);

    if(this->ep != -1)
        this->zobristHash ^= zobristNumbers[zEpFileIndex + (this->ep % 8)];

    if(ep != -1)
        this->zobristHash ^= zobristNumbers[zEpFileIndex + (ep % 8)];

    this->zobristHash ^= zobristNumbers[zCastleRightsIndex + this->castleRights];
    this->zobristHash ^= zobristNumbers[zCastleRightsIndex + castleRights];

    this->ep = ep;
    this->castleRights = castleRights;

    if(piece == King) {
        if(color == White) this->whiteKingSquare = m.from;
        if(color == Black) this->blackKingSquare = m.from;
    }

    this->deletePieceInBB(piece, color, m.to);

    if(m.prom) {
        this->squares[m.to] = (Pawn | color);
        this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Pawn, color, m.from)];
    }

    this->addPieceInBB((this->squares[m.to] ^ color), color, m.from);
    if(m.capture && !m.ep) this->addPieceInBB(otherPiece, otherColor, m.to);

    this->zobristHash ^= zobristNumbers[zPieceSquareIndex(piece, color, m.to)];
    if(m.capture && !m.ep) this->zobristHash ^= zobristNumbers[zPieceSquareIndex(otherPiece, otherColor, m.to)];
    if(!m.prom) this->zobristHash ^= zobristNumbers[zPieceSquareIndex(piece, color, m.from)];


    this->squares[m.from] = this->squares[m.to];
    this->squares[m.to] = m.capture;

    if(m.castle) {
        if(m.to == c1) {
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, White, a1)];
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, White, d1)];
            this->deletePieceInBB(Rook, color, d1);
            this->addPieceInBB(Rook, color, a1);

            swap(this->squares[a1], this->squares[d1]);
        } else if(m.to == g1) {
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, White, h1)];
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, White, f1)];
            this->deletePieceInBB(Rook, color, f1);
            this->addPieceInBB(Rook, color, h1);

            swap(this->squares[f1], this->squares[h1]);
        } else if(m.to == g8) {
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, Black, f8)];
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, Black, h8)];
            this->deletePieceInBB(Rook, color, f8);
            this->addPieceInBB(Rook, color, h8);

            swap(this->squares[f8], this->squares[h8]);
        } else {
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, Black, a8)];
            this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Rook, Black, d8)];
            this->deletePieceInBB(Rook, color, d8);
            this->addPieceInBB(Rook, color, a8);

            swap(this->squares[a8], this->squares[d8]);
        }
    }

    if(m.ep) {
        int capturedPawnSquare = m.to+(color == White ? South : North);

        this->addPieceInBB(Pawn, otherColor, capturedPawnSquare);
        this->zobristHash ^= zobristNumbers[zPieceSquareIndex(Pawn, otherColor, capturedPawnSquare)];

        this->squares[capturedPawnSquare] = (otherColor | Pawn);
        this->squares[m.to] = Empty;
    }

    this->turn ^= (Black | White);
    this->zobristHash ^= zobristNumbers[zBlackTurnIndex];
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
int moveGenTest(int depth) {
    if(depth == 0) return 1;

    vector<Move> moves = board.GenerateLegalMoves();
    int numPos = 0;

    for(Move m: moves) {
        int ep = board.ep;
        int castleRights = board.castleRights;

        board.makeMove(m);
        int mv = moveGenTest(depth-1);

        numPos += mv;

        board.unmakeMove(m, ep, castleRights);
    }
    return numPos;
}
