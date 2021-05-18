#include <bits/stdc++.h>

#include "Evaluate.h"
#include "Board.h"
#include "Search.h"

using namespace std;

// considers captures first, sorting them by the difference between the captured and capturing piece
int moveScore(Move m) {
    int score = 0;

    if(m.capture) score += 1000000;

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
        bool castleRights[4] = {board.castleWK, board.castleWQ,
                                board.castleBK, board.castleBQ};
        board.makeMove(m);
        int eval = -quiesce(-beta, -alpha);
        board.unmakeMove(m, ep, castleRights);

        if(eval >= beta)
            return beta;
        alpha = max(alpha, eval);
    }
    return alpha;
}


// negamax algorithm with alpha-beta pruning
int Search(int depth, int alpha, int beta) {
    if(depth == 0) {
        return quiesce(alpha, beta);
    }

    Move bestMove;

    // we already searched this node with a better depth, so we return the result from the table
    if(transpositionTable.find(board.zobristHash) != transpositionTable.end()) {
        int oldDepth = transpositionTable[board.zobristHash].second.first;
        if(oldDepth >= depth) {
            return transpositionTable[board.zobristHash].second.second;
        }
    }

    vector<Move> moves = board.GenerateLegalMoves();

    // game is over
    if(moves.size() == 0) {
        if(board.isInCheck())
            return (board.turn == White ? -1000000 : 1000000);
        return 0;
    }

    // sorting the moves by score in order to prune more branches by considering the potentially better moves first;
    sort(moves.begin(), moves.end(), compareMoves);

    int bestEval = -1000000;
    for(Move m: moves) {
        int ep = board.ep;
        bool castleRights[4] = {board.castleWK, board.castleWQ,
                                board.castleBK, board.castleBQ};

        board.makeMove(m);
        int eval = -Search(depth-1, -beta, -alpha);
        board.unmakeMove(m, ep, castleRights);

        if(eval >= beta)
            return eval; // fail-soft beta-cutoff
        if(eval > bestEval) {
            bestMove = m;
            bestEval = eval;
            alpha = max(alpha, eval);
        }
    }

    transpositionTable[board.zobristHash] = {bestMove, {depth, bestEval * (board.turn == White ? 1 : -1)}};
    return bestEval;
}
