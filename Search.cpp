#include <bits/stdc++.h>

#include "Evaluate.h"
#include "Board.h"
#include "Search.h"
#include "TranspositionTable.h"
#include "MagicBitboards.h"

using namespace std;

const int inf = 1000000;
const Move noMove = {-1,-1,0,0,0,0};

vector<Move> bestPV;
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
    // if both are captures just compare them using the other function
    if(a.capture && b.capture) return cmpCaptures(a, b);

    // prioritize captures over 'quiet' moves
    if(a.capture) return true;
    if(b.capture) return false;

    return false;
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

// quick search to prove if a node is worth doing a complete search on
int nullWindowSearch(int beta, int depth) {
    int alpha = beta-1;
    if(depth == 0) return quiesce(alpha, beta);

    vector<Move> moves = board.GenerateLegalMoves();
    sortMoves(moves, retrieveBestMove());

    for(Move m: moves) {
        board.makeMove(m);
        int score = -nullWindowSearch(-alpha, depth-1);
        board.unmakeMove(m);

        if(score >= beta) return beta;
    }
    return alpha;
}

// negamax algorithm with alpha-beta pruning
int alphaBeta(int alpha, int beta, int depth, int distToRoot, vector<Move> &PV) {
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
    if(beta > mateScore) beta = mateScore;
    if(alpha >= beta) return alpha;

    // game over
    if(isDraw()) return 0; //insufficient material

    vector<Move> moves = board.GenerateLegalMoves();
    if(moves.size() == 0) {
        if(isInCheck)
            return -mateScore;
        return 0; //stalemate
    }

    // first move should always be the hashed move or the best move found in previous iterations
    Move firstMove;
    if(distToRoot == 0) firstMove = bestMove;
    else firstMove = retrieveBestMove();
    sortMoves(moves, firstMove);

    int hashScore = ProbeHash(depth, alpha, beta, &ttMove);
    if(hashScore != valUnknown) {
        Move pvMove = retrieveBestMove();
        if(pvMove.from != noMove.from) PV.push_back(pvMove);
        return hashScore;
    }

    if(depth == 0) return quiesce(alpha, beta);
    Move currBestMove = noMove;
    bool doPVSearch = true;

    for(Move m: moves) {
        board.makeMove(m);

        // principal variation search
        // we do a full search only until we find a move that raises alpha and we consider it to be the best
        // for the rest of the moves we start with a quick (null window) search
        // and only if the move has potential to be the best, we do a full search
        int score;
        vector<Move> childPV;
        if(doPVSearch) {
            score = -alphaBeta(-beta, -alpha, depth-1, distToRoot+1, childPV);
        } else {
            score = -nullWindowSearch(-alpha, depth-1);
            if(score > alpha && score < beta)
                score = -alphaBeta(-beta, -alpha, depth-1, distToRoot+1, childPV);
        }
        board.unmakeMove(m);

        if(timeOver) return 0;

        if(score >= beta) {
            RecordHash(depth, beta, hashFBeta, currBestMove);
            return beta;
        }

        if(score > alpha) {
            hashFlag = hashFExact;
            alpha = score;
            doPVSearch = false;

            if(distToRoot == 0) bestMove = m;
            currBestMove = m;

            PV.clear();
            PV.push_back(currBestMove);
            copy(childPV.begin(), childPV.end(), back_inserter(PV));
        }
    }

    RecordHash(depth, alpha, hashFlag, currBestMove);
    return alpha;
}

const int aspirationWindow = 50;
pair<Move, int> Search() {
    startTime = clock();
    bestMove = noMove;
    timeOver = false;
    nodesSearched = 0;
    bestPV.clear();

    vector<Move> newPV;

    int alpha = -inf, beta = inf;

    int eval = alphaBeta(-inf, inf, 1, 0, newPV);
    bestPV = newPV;
    for(int depth = 2; depth <= maxDepth; ) {

        newPV.clear();
        int curEval = alphaBeta(alpha, beta, depth, 0, newPV);

        if(timeOver) {
            cout << "depth:" << depth << '\n';
            break;
        }
        bestPV = newPV;

        // if we fall outside the window, we do a full width search with the same depth
        if(curEval <= alpha || curEval >= beta) {
            alpha = -inf;
            beta = inf;
            continue;
        }

        eval = curEval;
        alpha = eval-aspirationWindow; // increase window for next iteration
        beta = eval+aspirationWindow;
        depth++; // increase depth only if we are inside the window
    }

    cout << "nodes:" << nodesSearched << '\n';
    cout << "PV: ";
    for(Move m: bestPV) cout << moveToString(m) << ' ';
    cout << '\n';
    return {bestMove, eval};
}
