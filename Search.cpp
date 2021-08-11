#include <bits/stdc++.h>

#include "Evaluate.h"
#include "Board.h"
#include "Search.h"
#include "TranspositionTable.h"
#include "MagicBitboards.h"

using namespace std;

const int inf = 1000000;
const Move noMove = {-1,-1,0,0,0,0};

Move bestMove = noMove;

const int mateEval = inf-1;
const int MATE_THRESHOLD = mateEval/2;

clock_t startTime;
const int timePerMove = 30;
const int maxDepth = 100;

// when nodes searched reach a certain number, we check for time over
int nodesSearched = 0;
bool timeOver = false;
Move ttMove = noMove;

// draw by insufficient material
bool isDraw() {
    if(board.queensBB | board.rooksBB | board.pawnsBB) return false;

    if((board.knightsBB | board.bishopsBB) == 0) return true; // king vs king

    int whiteBishops = popcount(board.bishopsBB | board.whitePiecesBB);
    int blackBishops = popcount(board.bishopsBB | board.blackPiecesBB);
    int whiteKnights = popcount(board.knightsBB | board.whitePiecesBB);
    int blackKnights = popcount(board.knightsBB | board.blackPiecesBB);

    if(whiteKnights + blackKnights + whiteBishops + blackBishops == 1) return true; // king and minor piece vs king

    if(whiteKnights + blackKnights == 0 && whiteBishops == 1 && blackBishops == 1) {
        int lightSquareBishops = popcount(lightSquaresBB & board.bishopsBB);
        if(lightSquareBishops == 0 || lightSquareBishops == 2) return true; // king and bishop vs king and bishop with same color bishops
    }

    return false;
}

bool areEqual(Move a, Move b) {
    return ((a.from == b.from) && (a.to == b.to));
}

bool cmpCaptures(Move a, Move b) {
    int otherColor = (board.turn ^ (Black | White));

    int scoreA = (a.capture ^ otherColor);
    scoreA -= (board.squares[a.from] ^ board.turn);

    int scoreB = (b.capture ^ otherColor);
    scoreB -= (board.squares[b.from] ^ board.turn);

    return (scoreA > scoreB);
}

bool cmpMoves(Move a, Move b) {

    // first move should always be the current best
    if(areEqual(a, bestMove)) return true;
    if(areEqual(b, bestMove)) return false;

    // if both are captures just compare them using the other function
    if(a.capture && b.capture) return cmpCaptures(a, b);

    // prioritize captures over 'quiet' moves
    if(a.capture) return true;
    if(b.capture) return false;

    return false;
}


// only searching for captures at the end of a regular search in order to ensure the engine won't miss obvious tactics
int quiesce(int alpha, int beta) {
    if(!(nodesSearched & 4095)) {
        if((clock() - startTime) / CLOCKS_PER_SEC >= timePerMove)
            timeOver = true;
    }
    if(timeOver) return 0;
    nodesSearched++;

    vector<Move> moves = board.GenerateLegalMoves();
    sort(moves.begin(), moves.end(), cmpCaptures);

    int standPat = Evaluate();
    if(standPat >= beta)
        return beta;
    alpha = max(alpha, standPat);

    for(Move m : moves)  {
        if(!m.capture) continue;

        board.makeMove(m);
        int score = -quiesce(-beta, -alpha);
        board.unmakeMove(m);

        if(timeOver) return 0;

        if(score > alpha) {
            if(score >= beta)
                return beta;
            alpha = score;
        }
    }
    return alpha;
}

// negamax algorithm with alpha-beta pruning
int alphaBeta(int alpha, int beta, int depth, int distToRoot) {
    if(!(nodesSearched & 4095)) {
        if((clock() - startTime) / CLOCKS_PER_SEC >= timePerMove)
            timeOver = true;
    }
    if(timeOver) return 0;
    nodesSearched++;

    int hashFlag = hashFAlpha;

    bool isInCheck = board.isInCheck();

    // increase the depth if king is in check
    if(isInCheck) depth++;

    // mate distance pruning
    int mateScore = mateEval-distToRoot;

    if(alpha < -mateScore) alpha = -mateScore;
    if(beta > mateScore - 1) beta = mateScore - 1;
    if(alpha >= beta) return alpha;

    // game over
    if(isDraw()) return 0; //insufficient material

    vector<Move> moves = board.GenerateLegalMoves();
    if(moves.size() == 0) {
        if(isInCheck)
            return -mateScore;
        return 0; //stalemate
    }
    sort(moves.begin(), moves.end(), cmpMoves);

    int hashScore = ProbeHash(depth, alpha, beta, &ttMove);
    if(hashScore != valUnknown)
        return hashScore;

    if(depth == 0) return quiesce(alpha, beta);
    Move currBestMove = noMove;

    for(Move m: moves) {
        board.makeMove(m);
        int score = -alphaBeta(-beta, -alpha, depth-1, distToRoot+1);
        board.unmakeMove(m);

        if(timeOver) return 0;

        if(score >= beta) {
            RecordHash(depth, beta, hashFBeta, currBestMove);
            return beta;
        }

        if(score > alpha) {
            hashFlag = hashFExact;
            alpha = score;

            if(distToRoot == 0) bestMove = m;
            currBestMove = m;
        }
    }

    RecordHash(depth, alpha, hashFlag, currBestMove);
    return alpha;
}

pair<Move, int> Search() {
    startTime = clock();
    bestMove = noMove;
    timeOver = false;
    nodesSearched = 0;

    int eval = alphaBeta(-inf, inf, 1, 0);
    for(int depth = 2; depth <= maxDepth; depth++) {
        int curEval = alphaBeta(-inf, inf, depth, 0);

        // time runs out
        if(timeOver) {
            cout << "depth:" << depth << '\n';
            break;
        }
        eval = curEval;
    }
    return {bestMove, eval};
}
