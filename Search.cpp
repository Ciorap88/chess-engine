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

// ordering the moves based on material gain
bool cmpMoves(Move a, Move b) {
    int otherColor = (board.turn ^ (Black | White));

    // initial score is 0, if the move is a capture we favor the moves that capture 
    // a more valuable piece with a less valuable one
    int scoreA = 0;
    if(a.capture) {
        scoreA += pieceValues[(a.capture ^ otherColor)];
        scoreA -= pieceValues[(board.squares[a.from] ^ board.turn)];
    }

    int scoreB = 0;
    if(b.capture) {
        scoreB += pieceValues[(b.capture ^ otherColor)];
        scoreB -= pieceValues[(board.squares[b.from] ^ board.turn)];
    }

    // if the move is a pawn promotion we add the material gain to the score
    if(a.prom) scoreA += pieceValues[a.prom] - pieceValues[Pawn];
    if(b.prom) scoreB += pieceValues[b.prom] - pieceValues[Pawn];

    return (scoreA > scoreB);
}

void sortMoves(vector<Move> &moves, Move PVMove) {
    // if there is not a pv move we just sort them normally
    if(areEqual(PVMove, noMove)) {
        sort(moves.begin(), moves.end(), cmpMoves);
        return;
    }

    // if there is a pv move we put it first and then sort the rest
    vector<Move> sorted, nonPVMoves;
    for(Move m: moves) {
        if(areEqual(PVMove, m)) sorted.push_back(m);
        else nonPVMoves.push_back(m);
    }
    sort(nonPVMoves.begin(), nonPVMoves.end(), cmpMoves);
    for(Move m: nonPVMoves) sorted.push_back(m);

    moves = sorted;
}

// ---quiescence search---
// only searching for captures at the end of a regular search in order to ensure the engine won't miss obvious tactics
int quiesce(int alpha, int beta) {
    if(!(nodesSearched & 4095)) {
        if((clock() - startTime) / CLOCKS_PER_SEC >= timePerMove)
            timeOver = true;
    }
    if(timeOver) return 0;
    nodesSearched++;

    if(isDraw()) return 0;

    vector<Move> moves = board.GenerateLegalMoves();
    sortMoves(moves, noMove);

    int standPat = Evaluate();
    if(standPat >= beta)
        return beta;

    // ---delta pruning---
    // we test if the greatest material swing is enough to raise alpha
    // if it isn't, then the position is hopeless so searching deeper won't improve it
    int delta = 975 + 875 + 200; // capturing a queen + promoting a pawn to a queen + safety margin
    if(delta + standPat <= alpha) return alpha;

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

// alpha-beta algorithm with a lot of enhancements
int alphaBeta(int alpha, int beta, int depth, int distToRoot, bool doNull) {
    assert(depth >= 0);

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

    // ---mate distance pruning---
    // if we find mate, we shouldn't look for a better move
    int mateScore = mateEval-distToRoot;

    if(alpha < -mateScore) alpha = -mateScore;
    if(beta > mateScore) beta = mateScore;
    if(alpha >= beta) return alpha;

    if(isDraw()) return 0; //insufficient material
    vector<Move> moves = board.GenerateLegalMoves();
    if(moves.size() == 0) {
        if(isInCheck)
            return -mateScore; //checkmate
        return 0; //stalemate
    }

    // ---move ordering---
    // we sort the moves so that we can achieve cutoffs faster, so we don't need to search all the moves too deep
    // first move should always be the hashed move or the best move found in previous iterations
    Move firstMove;
    if(distToRoot == 0 && bestMove.from != -1) firstMove = bestMove;
    else firstMove = retrieveBestMove();
    sortMoves(moves, firstMove);

    // retrieving the hashed move and evaluation if there is any
    int hashScore = ProbeHash(depth, alpha, beta);
    if(hashScore != valUnknown)
        return hashScore;

    if(depth == 0) return quiesce(alpha, beta);
    Move currBestMove = noMove;
    bool doPVSearch = true;

    // ---null move pruning---
    // if our position is good, we can pass the turn to the opponent
    // and if that doesn't wreck our position, we don't need to search further
    if((isInCheck == false) && distToRoot && (depth >= 3) && (Evaluate() >= beta) && doNull && (gamePhase >= endgameMaterial)) {
        board.makeMove(noMove);

        int R = (depth > 6 ? 3 : 2);
        int score = -alphaBeta(-beta, -beta+1, depth-R-1, distToRoot+1, false);

        board.unmakeMove(noMove);

        if(timeOver) return 0;
        if(score >= beta) return beta;
    }

    int movesSearched = 0;
    for(Move m: moves) {
        if(alpha >= beta) return alpha;

        board.makeMove(m);
        int score;

        // ---late move reduction---
        // we do full searches only for the first moves, and then do a reduced search
        // if the move is potentially good, we do a full search instead
        int reductionDepth = 0;
        if(movesSearched > 3 && !m.capture && !m.prom && !isInCheck && depth > 4 && !board.isInCheck()) {
            reductionDepth = 1;
            if(movesSearched > 8) reductionDepth ++;

            depth -= reductionDepth;
        } 

    pvSearch:
        // ---principal variation search---
        // we do a full search only until we find a move that raises alpha and we consider it to be the best
        // for the rest of the moves we start with a quick (null window - beta = alpha+1) search
        // and only if the move has potential to be the best, we do a full search
        if(doPVSearch) {
            score = -alphaBeta(-beta, -alpha, depth-1, distToRoot+1, true);
        } else {
            score = -alphaBeta(-alpha-1, -alpha, depth-1, distToRoot+1, true);
            if(score > alpha && score < beta)
                score = -alphaBeta(-beta, -alpha, depth-1, distToRoot+1, true);
        }

        // move can be good, we do a full depth search
        if(reductionDepth && score > alpha) {
            depth += reductionDepth;
            reductionDepth = 0;

            goto pvSearch;
        }

        board.unmakeMove(m);
        movesSearched++;

        if(timeOver) return 0;

        if(score >= beta) {
            RecordHash(depth, beta, hashFBeta, currBestMove);
            return beta;
        }

        if(score > alpha) {
            hashFlag = hashFExact;
            alpha = score;
            doPVSearch = false;

            currBestMove = m;
        }
        
    }

    if(!timeOver && distToRoot == 0) bestMove = currBestMove;
    RecordHash(depth, alpha, hashFlag, currBestMove);
    return alpha;
}

const int aspIncrease = 50;
pair<Move, int> Search() {
    startTime = clock();
    bestMove = noMove;
    timeOver = false;
    nodesSearched = 0;

    int alpha = -inf, beta = inf;
    int eval = 0;

    //---iterative deepening---
    // we start with a depth 1 search and then we increase the depth by 1 every time
    // this helps manage the time because at any point the engine can return the best move found so far
    // also it helps improve move ordering by memorizing the best move that we can search first in the next iteration
    for(int depth = 1; depth <= maxDepth; ) {

        int curEval = alphaBeta(alpha, beta, depth, 0, false);

        if(timeOver) {
            cout << "depth:" << depth << '\n';
            showPV(depth-1);
            break;
        }

        // ---aspiration window---
        // we start with a full width search (alpha = -inf and beta = inf), modifying them accordingly
        // if we fall outside the current window, we do a full width search with the same depth
        // and if we stay inside the window, we only increase it by a small number, so that we can achieve more cutoffs
        if(curEval <= alpha || curEval >= beta) {
            alpha = -inf;
            beta = inf;
            continue;
        }

        eval = curEval;
        alpha = eval-aspIncrease; // increase window for next iteration
        beta = eval+aspIncrease;
        depth++; // increase depth only if we are inside the window
    }

    cout << "nodes:" << nodesSearched << '\n';
    return {retrieveBestMove(), eval};
}
