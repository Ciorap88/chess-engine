#include <bits/stdc++.h>

using namespace std;

typedef unsigned long long U64;

struct Move {
    int from, to, capture;
    bool ep, castle;
    int prom;
};

U64 bits[64];

const string startingPos = "rnbqkbnr/ppppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
vector<int> piecesDirs[8];
vector<int> knightTargetSquares[64];

enum Piece {
    Empty = 0,
    Pawn = 1,
    Knight = 2,
    Bishop = 3,
    Rook = 4,
    Queen = 5,
    King = 6,

    White = 8,
    Black = 16
};

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

string square(int x) {
    string s;
    s += ('a'+x%8);
    s += ('1'+x/8);
    return s;
}

void Init() {
    for(int i = 0; i < 64; i++)
        bits[i] = (1LL << i);

    piecesDirs[Bishop] = {NorthEast, NorthWest, SouthEast, SouthWest};
    piecesDirs[Rook] = {East, West, North, South};
    piecesDirs[King] = piecesDirs[Queen] = {NorthEast, NorthWest, SouthEast, SouthWest, East, West, North, South};



    for(int i = 0; i < 64; i++) {
        int File = i%8, Rank = i/8;

        if(File > 1) {
            if(Rank > 0) knightTargetSquares[i].push_back(i-10);
            if(Rank < 7) knightTargetSquares[i].push_back(i+6);
        }
        if(File < 6) {
            if(Rank > 0) knightTargetSquares[i].push_back(i-6);
            if(Rank < 7) knightTargetSquares[i].push_back(i+10);
        }
        if(Rank > 1) {
            if(File > 0) knightTargetSquares[i].push_back(i-17);
            if(File < 7) knightTargetSquares[i].push_back(i-15);
        }
        if(Rank < 6) {
            if(File > 0) knightTargetSquares[i].push_back(i+15);
            if(File < 7) knightTargetSquares[i].push_back(i+17);
        }
    }
}

/// returns the direction of the move if any, or 0
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

}

class Board {
public:
    int squares[64], turn;
    U64 pawns, knights, bishops, rooks, queens, kings;
    U64 blackPieces, whitePieces;

    int whiteKingSquare, blackKingSquare;
    int ep; /// en passant square
    bool castleWK, castleWQ, castleBK, castleBQ; /// castling rights
    /// add pieces array to lookup all the pieces faster

    void LoadFenPos(string fen) {
        unordered_map<char, int> pieceSymbols = {{'p', Pawn}, {'n', Knight},
        {'b', Bishop}, {'r', Rook}, {'q', Queen}, {'k', King}};
        int index = 0; /// index of character after the first space in the fen string

        int File = 0, Rank = 7;
        for(char symbol: fen) {
            index++;
            if(symbol == ' ') break;
            if(symbol == '/') {
                Rank--;
                File = 0;
            } else {
                /// if it is a digit skip the squares
                if(symbol-'0' >= 1 && symbol-'0' <= 8) {
                    for(int i = 0; i < (symbol-'0'); i++) {
                        this->squares[Rank*8 + File] = 0;
                        File++;
                    }
                } else {
                    int color = ((symbol >= 'A' && symbol <= 'Z') ? White : Black);
                    int type = pieceSymbols[tolower(symbol)];

                    int currBit = bits[Rank*8 + File];

                    if(color == White) this->whitePieces |= currBit;
                    if(color == Black) this->blackPieces |= currBit;

                    if(type == Pawn) this->pawns |= currBit;
                    if(type == Knight) this->knights |= currBit;
                    if(type == Bishop) this->bishops |= currBit;
                    if(type == Rook) this->rooks |= currBit;
                    if(type == Queen) this->queens |= currBit;
                    if(type == King) {
                        this->kings |= currBit;
                        if(color == White) this->whiteKingSquare = Rank*8 + File;
                        if(color == Black) this->blackKingSquare = Rank*8 + File;
                    }

                    this->squares[Rank*8 + File] = (color | type);
                    File++;
                }
            }
        }

        /// get castling rights, ep square and turn from the end of the string
        this->turn = (fen[index] == 'w' ? White : Black);
        index += 2;
        this->castleBK = this->castleBQ = false;
        this->castleWK = this->castleWQ = false;

        while(fen[index] != ' '){
            if(fen[index] == 'K') this->castleWK = true;
            if(fen[index] == 'Q') this->castleWQ = true;
            if(fen[index] == 'k') this->castleBK = true;
            if(fen[index] == 'q') this->castleBQ = true;
            index++;
        }
        index++;
        if(fen[index] != '-') {
            this->ep = (fen[index]-'a') + 8*(fen[index+1]-'1');
        } else {
            this->ep = -1;
        }
    }

    vector<Move> GeneratePseudoLegalMoves() {
        int color = this->turn;
        int otherColor = (color ^ (Black | White));

        vector<Move> moves;
        for(int i = 0; i < 64; i++) {
            if(this->squares[i] == Empty || this->squares[i] & color == 0) continue;
            int piece = (this->squares[i]) - color;
            int Rank = i/8;
            int File = i%8;

            /// moves for sliding pieces
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

            /// moves, captures and promotions for pawns
            if(piece == Pawn) {
                int dir = (color == White ? North : South);
                int startRank = (color == White ? 1 : 6);
                int promRank = (color == White ? 7 : 0);
                vector<int> promPieces = {Knight, Bishop, Rook, Queen};

                /// 2 square move at the beginning
                if(Rank == startRank && this->squares[i+dir] == Empty && this->squares[i+2*dir] == Empty) {
                    moves.push_back({i, i+2*dir, 0, 0, 0, 0});
                }

                /// normal moves and promotions
                if(Rank != promRank && this->squares[i+dir] == Empty) {
                    if((i+dir)/8 == promRank) {
                        for(auto pc: promPieces)
                            moves.push_back({i, i+dir, 0, 0, 0 , pc});
                    } else {
                        moves.push_back({i, i+dir, 0, 0, 0 , 0});
                    }
                }

                /// captures and capture-promotions
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

            /// moves for knights
            if(piece == Knight) {
                for(auto sq: knightTargetSquares[i]) {
                    if((this->squares[sq] & color) == 0) {
                        moves.push_back({i, sq, this->squares[sq], 0, 0, 0});
                    }
                }
            }

            /// moves for kings
            if(piece == King) {
                for(int dir: piecesDirs[King]) {
                    if(isInBoard(i, dir) && (this->squares[i+dir] & color) == 0) {
                        moves.push_back({i, i+dir, this->squares[i+dir], 0, 0, 0});
                    }
                }
            }
        }

        /// castles
        if(castleWK && this->squares[5] == Empty && this->squares[6] == Empty)
            moves.push_back({4, 6, 0, 0, 1, 0});
        if(castleWQ && this->squares[1] == Empty && this->squares[2] == Empty && this->squares[3] == Empty)
            moves.push_back({4, 2, 0, 0, 1, 0});
        if(castleBK && this->squares[61] == Empty && this->squares[62] == Empty)
            moves.push_back({60, 62, 0, 0, 1, 0});
        if(castleBQ && this->squares[57] == Empty && this->squares[58] == Empty && this->squares[59] == Empty)
            moves.push_back({60, 58, 0, 0, 1, 0});

        /// en passant
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

    bool isInCheck() {
        int color = this->turn;
        int otherColor = (color ^ (Black | White));

        int kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);

        for(auto sq: knightTargetSquares[kingSquare])
            if(this->squares[sq] == (otherColor | Knight))
                return true;

        vector<int> pawnDirs;
        if(color == White) pawnDirs = {NorthWest, NorthEast};
        if(color == Black) pawnDirs = {SouthWest, SouthEast};

        for(int dir: pawnDirs)
            if(isInBoard(kingSquare, dir) && this->squares[kingSquare+dir] == (Pawn | otherColor))
                return true;

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

    vector<Move> GenerateLegalMoves() {
        int color = this->turn;
        int otherColor = (color ^ (Black | White));

        int kingSquare = (color == White ? this->whiteKingSquare : this->blackKingSquare);
        int otherKingSquare = (otherColor == White ? this->whiteKingSquare : this->blackKingSquare);

        vector<Move> potentialMoves = this->GeneratePseudoLegalMoves();

        /// remove the king before calculating attacked squares by opponent's sliding pieces
        this->squares[kingSquare] = Empty;

        vector<int> attackedSquares(64, 0), checkingPieces;

        ///attacked squares for sliding pieces
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

        /// squares attacked by knights
        for(int i = 0; i < 64; i++) {
            if(this->squares[i] == (Knight | otherColor))
            for(auto j: knightTargetSquares[i]) {
                attackedSquares[j] ++;
                if(j == kingSquare) {
                    checkingPieces.push_back(i);
                }
            }
        }

        /// squares attacked by the opponent king
        for(int dir: piecesDirs[King]) {
            if(isInBoard(otherKingSquare, dir)) {
                attackedSquares[otherKingSquare+dir]++;
            }
        }

        /// squares attacked by pawns
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

        /// calculate the checking piece square
        /// and checking ray if the checking piece is a sliding piece
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

        /// finding the absolute pins
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
                            if(kingRay.count(j)) { /// the intersection of the king ray and the other piece ray
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
            } else if(checkingPieces.size() > 1) { /// if in double check, we can only move the king
                continue;
            } else if(checkingPieces.size() == 1) { /// if in single check, we can also intercept the check or capture the checking piece
                if(((m.ep && checkingPieces[0] == m.to + (color == White ? South : North)) || checkRay.count(m.to)) && !pinned.count(m.from))
                /// we can capture the checking pawn en passant if it is the only checking piece
                    moves.push_back(m);
            } else if(pinned.count(m.from)) {
                if(abs(direction(m.from, m.to)) == abs(pinDirection[m.from]))
                    moves.push_back(m);
            } else {
                moves.push_back(m);
            }
        }

        /// en passant are the last checked pseudo legal moves so we know they are at the back of the vector if they exist
        while(moves.size() && moves.back().ep) {
            epMoves.push_back(moves.back());
            moves.pop_back();
        }

        ///before ep there are the castle moves so we do the same
        vector<Move> castleMoves;
        while(moves.size() && moves.back().castle) {
            castleMoves.push_back(moves.back());
            moves.pop_back();
        }

        /// manual check for the legality of castle moves
        for(Move m: castleMoves) {
            int first = min(m.from, m.to);
            int second = max(m.from, m.to);

            bool ok = true;

            for(int i = first; i <= second; i++)
                if(attackedSquares[i])
                    ok = false;

            if(ok) moves.push_back(m);
        }

        /// ep horizontal pin
        for(Move enPassant: epMoves) { /// en passant moves that are pseudo legal and passed the previous tests
            int dir = direction(enPassant.from, enPassant.to);

            /// go in the ray direction from the 2 pawns and see if we find a king and an opposite colored queen/rook
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

    void makeMove(Move m) {
        int color = (this->squares[m.from] & (Black | White));
        int piece = (this->squares[m.from] ^ color);

        this->squares[m.to] = this->squares[m.from];
        this->squares[m.from] = Empty;

        /// promote pawn
        if(m.prom) this->squares[m.to] = (m.prom | color);

        /// remove both castling rights if the king moves
        if(piece == King) {
            if(color == White) {
                this->castleWK = this->castleWQ = false;
                this->whiteKingSquare = m.to;
            }
            if(color == Black) {
                this->castleBK = this->castleBQ = false;
                this->blackKingSquare = m.to;
            }
        }

        /// remove the respective castling right if a rook moves or gets captured
        if(m.from == 0 || m.to == 0) this->castleWQ = false;
        if(m.from == 7 || m.to == 7) this->castleWK = false;
        if(m.from == 56 || m.to == 56) this->castleBQ = false;
        if(m.from == 63 || m.to == 63) this->castleBK = false;

        /// move the rook if castle
        if(m.castle) {
            if(m.to == 2) {
                swap(this->squares[0], this->squares[3]);
            } else if(m.to == 6) {
                swap(this->squares[7], this->squares[5]);
            } else if(m.to == 62) {
                swap(this->squares[61], this->squares[63]);
            } else {
                swap(this->squares[56], this->squares[59]);
            }
        }
        /// remove the captured pawn if en passant
        if(m.ep) {
            int capturedPawnSquare = m.to+(color == White ? South : North);
            this->squares[capturedPawnSquare] = Empty;
        }

        /// update en passant target square
        this->ep = -1;
        if(piece == Pawn && abs(m.from-m.to) == 16) {
            this->ep = m.to+(color == White ? -8 : 8);
        }

        /// switch the turn
        this->turn ^= (Black | White);
    }

    /// basically the inverse of makeMove but we need to memorize the castling right and ep square before the move
    void unmakeMove(Move m, int ep, bool castlingRights[4]) {
        int color = (this->squares[m.to] & (Black | White));
        int otherColor = (color ^ (Black | White));
        int piece = (this->squares[m.to] ^ color);

        this->ep = ep;
        this->castleWK = castlingRights[0];
        this->castleWQ = castlingRights[1];
        this->castleBK = castlingRights[2];
        this->castleBQ = castlingRights[3];

        if(piece == King) {
            if(color == White) this->whiteKingSquare = m.from;
            if(color == Black) this->blackKingSquare = m.from;
        }

        if(m.prom) this->squares[m.to] = (Pawn | color);

        this->squares[m.from] = this->squares[m.to];
        this->squares[m.to] = m.capture;

        if(m.castle) {
            if(m.to == 2) {
                swap(this->squares[0], this->squares[3]);
            } else if(m.to == 6) {
                swap(this->squares[7], this->squares[5]);
            } else if(m.to == 62) {
                swap(this->squares[61], this->squares[63]);
            } else {
                swap(this->squares[56], this->squares[59]);
            }
        }

        if(m.ep) {
            int capturedPawnSquare = m.to+(color == White ? South : North);
            this->squares[capturedPawnSquare] = (otherColor | Pawn);
            this->squares[m.to] = Empty;
        }

        this->turn ^= (Black | White);
    }
};

Board board;

bool putsKingInCheck(Move a) {
    bool check = false;

    int ep = board.ep;
    bool castleRights[4] = {board.castleWK, board.castleWQ,
                                board.castleBK, board.castleBQ};
    board.makeMove(a);
    if(board.isInCheck()) check = true;
    board.unmakeMove(a, ep, castleRights);

    return check;
}

int mxDepth = 5;
int moveGenTest(int depth) {
    if(depth == 0) return 1;

    vector<Move> moves = board.GenerateLegalMoves();
    int numPos = 0;

    for(Move m: moves) {
        int ep = board.ep;
        bool castleRights[4] = {board.castleWK, board.castleWQ,
                            board.castleBK, board.castleBQ};
        board.makeMove(m);
        int mv = moveGenTest(depth-1);
        if(depth == mxDepth)
            cout << square(m.from) << square(m.to) << ": " << mv << '\n';

        numPos += mv;

        board.unmakeMove(m, ep, castleRights);
    }
    return numPos;
}
