#include <bits/stdc++.h>

#include "Evaluate.h"
#include "Board.h"
#include "Search.h"

using namespace std;

const int inf = 1000000;
const Move noMove = {-1,-1,0,0,0,0};

const int mateEval = inf-1;
const int MATE_THRESHOLD = mateEval/2;

unordered_map<unsigned long long, pair<Move, pair<int, int> > > tt;

clock_t startTime;
const int timePerMove = 15;
const int maxDepth = 100;

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

bool cmpCaptures(Move a, Move b) {
    int otherColor = (board.turn ^ (Black | White));

    int scoreA = (a.capture ^ otherColor);
    scoreA -= (board.squares[a.from] ^ board.turn);

    int scoreB = (b.capture ^ otherColor);
    scoreB -= (board.squares[b.from] ^ board.turn);

    return (scoreA > scoreB);
}


// only searching for captures at the end of a regular search in order to ensure the engine won't miss any tactics
int quiesce(int alpha, int beta) {
    vector<Move> moves = board.GenerateLegalMoves();
    sort(moves.begin(), moves.end(), cmpCaptures);

    int standPat = Evaluate();
    if(standPat >= beta)
        return beta;
    alpha = max(alpha, standPat);

    for(Move m : moves)  {
        if((clock() - startTime) / CLOCKS_PER_SEC > timePerMove)
            break;

        if(!m.capture) continue;

        int ep = board.ep;
        int castleRights = board.castleRights;

        board.makeMove(m);
        int score = -quiesce(-beta, -alpha);
        board.unmakeMove(m, ep, castleRights);

        if(score >= beta)
            return beta;
        alpha = max(alpha, score);
    }
    return alpha;
}

// negamax algorithm with alpha-beta pruning
int alphaBeta(int alpha, int beta, int depth, int distToRoot) {
    int bestScore = -inf;
    Move bestMove = noMove;

    bool isInCheck = board.isInCheck();

    // game over
    if(isDraw()) return 0; //insufficient material

    vector<Move> moves = board.GenerateLegalMoves();
    if(moves.size() == 0) {
        if(isInCheck)
            return -(mateEval - distToRoot); // score is higher for a faster mate
        return 0; //stalemate
    }

    bool isStored = (tt.find(board.zobristHash) != tt.end());

    if(isStored && (tt[board.zobristHash].second.second >= depth || tt[board.zobristHash].second.first >= mateEval-distToRoot)) {
        return tt[board.zobristHash].second.first;
    }

    // increase the depth if king is in check because there are fewer moves to calculate
    if(isInCheck) depth++;

    if(depth == 0) return quiesce(alpha, beta);

    for(Move m: moves) {
        if((clock() - startTime) / CLOCKS_PER_SEC > timePerMove)
            break;

        int ep = board.ep;
        int castleRights = board.castleRights;

        board.makeMove(m);
        int score = -alphaBeta(-beta, -alpha, depth-1, distToRoot+1);

        if(score >= MATE_THRESHOLD) score--;
        if(score <= -MATE_THRESHOLD) score++;

        board.unmakeMove(m, ep, castleRights);

        if(score >= beta) return score;  // fail-soft beta-cutoff
        if(score > bestScore) {
            bestScore = score;
            bestMove = m;
            if(score > alpha) alpha = score;
        }
    }

    // update tt only if the program ran a complete search
    if((clock() - startTime) / CLOCKS_PER_SEC < timePerMove)
        tt[board.zobristHash] = {bestMove, {bestScore, depth}};

    return bestScore;
}

pair<Move, int> Search() {
    startTime = clock();

    for(int depth = 1; depth <= maxDepth; depth++) {

        // if the program finds mate, it shouldn't search further
        if(tt[board.zobristHash].second.first >= mateEval-depth)
            break;

        alphaBeta(-inf, inf, depth, 1);

        // time runs out
        if((clock() - startTime) / CLOCKS_PER_SEC > timePerMove) {
            cout << depth << ' ';
            break;
        }
    }
    return {tt[board.zobristHash].first, tt[board.zobristHash].second.first};
}
