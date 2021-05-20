#include <bits/stdc++.h>

#include "Evaluate.h"
#include "Board.h"
#include "Search.h"

using namespace std;

const int inf = 1000000;
const Move noMove = {-1,-1,0,0,0,0};

const int mateEval = inf-1;

// considers captures first, sorting them by the difference between the captured and capturing piece
int moveScore(Move m) {
    int score = 0;

    if(m.capture) score += inf;

    int capturedPieceVal = pieceValues[m.capture ^ (Black & White)];
    int capturingPieceVal = pieceValues[board.squares[m.from] ^ (Black & White)];

    score += (capturedPieceVal - capturingPieceVal);

    return score;
}

// function that compares 2 moves in order to sort them
bool compareMoves(Move a, Move b) {
    return (moveScore(a) > moveScore(b));
}

unordered_map<unsigned long long, pair<Move, pair<int, int> > > transpositionTable;

time_t startTime, currTime;
const double timePerMove = 30;
const int maxDepth = 100;

// only searching for captures at the end of a regular search in order to ensure the engine won't miss any tactics
int quiesce(int alpha, int beta) {
    int standPat = Evaluate();
    if(standPat >= beta)
        return beta;
    alpha = max(alpha, standPat);

    vector<Move> moves = board.GenerateLegalMoves();

    for(Move m : moves)  {
        if(!m.capture) continue;

        int ep = board.ep;
        int castleRights = board.castleRights;

        board.makeMove(m);
        int eval = -quiesce(-beta, -alpha);
        board.unmakeMove(m, ep, castleRights);

        if(eval >= beta)
            return beta;
        alpha = max(alpha, eval);
    }
    return alpha;
}

ofstream out ("res.txt");

// negamax algorithm with alpha-beta pruning
int AlphaBeta(int depth, int alpha, int beta) {
    vector<Move> moves = board.GenerateLegalMoves();

    // game is over
    if(moves.size() == 0) {
        if(board.isInCheck())
            return -mateEval;
        return 0;
    }

    time(&currTime);
    if(depth == 0 || (double)(currTime - startTime) > timePerMove) {
        return quiesce(alpha, beta);
    }

    Move bestMove = noMove;

    // we already searched this node with a better depth, so we return the result from the table
    if(transpositionTable.find(board.zobristHash) != transpositionTable.end()) {
        int oldDepth = transpositionTable[board.zobristHash].second.second;
        if(oldDepth >= depth) {
            return transpositionTable[board.zobristHash].second.first;
        }
    }

    // sorting the moves by score in order to prune more branches by considering the potentially better moves first;
    sort(moves.begin(), moves.end(), compareMoves);

    int bestEval = -inf;
    for(Move m: moves) {
        int ep = board.ep;
        int castleRights = board.castleRights;

        board.makeMove(m);
        int eval = -AlphaBeta(depth-1, -beta, -alpha);

        board.unmakeMove(m, ep, castleRights);

        if(eval >= beta)
            return eval;
        if(eval > bestEval) {
            bestEval = eval;
            bestMove = m;
            alpha = max(alpha, eval);
        }

        // if we find a checkmate we should do it
        if(eval == mateEval) {
            transpositionTable[board.zobristHash] = {bestMove, {eval, maxDepth}};
            return eval;
        }
    }

    transpositionTable[board.zobristHash] = {bestMove, {bestEval, depth}};
    return bestEval;
}

pair<Move, int> Search() {
    int eval = AlphaBeta(1, -inf, inf);

    time(&startTime);
    ios_base::sync_with_stdio(false);

    for(int depth = 2; depth <= maxDepth; depth++) {
        eval = AlphaBeta(depth, -inf, inf);

        time(&currTime);
        if((double)(currTime - startTime) > timePerMove) {
            cout << depth << '\n';
            return {transpositionTable[board.zobristHash].first, eval};
        }
    }
    return {transpositionTable[board.zobristHash].first, eval};
}
