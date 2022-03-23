#include <bits/stdc++.h>

#include "Evaluate.h"
#include "Board.h"
#include "Search.h"
#include "TranspositionTable.h"
#include "MagicBitboards.h"
#include "UCI.h"
#include "Moves.h"

using namespace std;

int killerMoves[200][2];

const int INF = 1000000;
const int NO_MOVE = -1;

int bestMove = NO_MOVE;

const int MATE_EVAL = INF-1;
const int MATE_THRESHOLD = MATE_EVAL/2;

int startTime, stopTime;
short maxDepth = 200;
bool infiniteTime;

int nodesSearched = 0;
int nodesQ = 0;
bool timeOver = false;

// store unique killer moves
void storeKiller(short ply, int move) {
    if(isCapture(move) || isPromotion(move)) return; // killer moves are by definition quiet moves

    // make sure the moves are different
    if(killerMoves[ply][0] != move) 
        killerMoves[ply][1] = killerMoves[ply][0];

    killerMoves[ply][0] = move;
}

// evaluating moves by material gain
int captureScore(int move) {
    int score = 0;

    // captured piece value - capturing piece value
    if(isCapture(move)) score += (PIECE_VALUES[getCapturedPiece(move)]-
                  PIECE_VALUES[getPiece(move)]);

    // material gained by promotion
    if(isPromotion(move)) score += PIECE_VALUES[getPromotionPiece(move)] - PIECE_VALUES[Pawn];

    return score;
}

bool cmpCapturesDesc(int a, int b) {
    return (captureScore(a) < captureScore(b));
}

bool cmpCapturesAsc(int a, int b) {
    return  (captureScore(a) > captureScore(b));
}

// ---move ordering---
void sortMoves(int *moves, unsigned char num, short ply) {
    // sorting in quiescence search
      if(ply == -1) {
          sort(moves, moves+num, cmpCapturesAsc);
          return;
      }

    int captures[256], nonCaptures[256];
    unsigned char nCaptures = 0, nNonCaptures = 0;

    // find pv move
    int pvMove = NO_MOVE;
    if(ply == 0) pvMove = bestMove;
    if(pvMove == NO_MOVE) pvMove = retrieveBestMove();

    // check legality of killer moves
    bool killerLegal[2] = {false, false};
    for(char i = 0; i < 2; i++)
        for(unsigned char idx = 0; idx < num; idx++)
            if(killerMoves[ply][i] == moves[idx])
                killerLegal[i] = true;

    // split the other moves into captures and non captures for easier sorting
    for(unsigned char idx = 0; idx < num; idx++) {
        if((moves[idx] == pvMove) || (moves[idx] == killerMoves[ply][0]) || (moves[idx] == killerMoves[ply][1]))
            continue;

        if(isCapture(moves[idx])) captures[nCaptures++] = moves[idx];
        else nonCaptures[nNonCaptures++] = moves[idx];
    }

    unsigned char newNum = 0; // size of sorted array

    // 1: add pv move
    if(pvMove != NO_MOVE) moves[newNum++] = pvMove;
    
    // 2: add captures sorted by MVV-LVA (only winning / equal captures first)
    sort(captures, captures+nCaptures, cmpCapturesDesc);
    while(nCaptures && captureScore(captures[nCaptures-1]) >= 0) {
        moves[newNum++] = captures[--nCaptures];
    }

    // 3: add killer moves
    for(char i = 0; i < 2; i++)
        if(killerLegal[i] && (killerMoves[ply][i] != NO_MOVE) && (killerMoves[ply][i] != pvMove)) 
            moves[newNum++] = killerMoves[ply][i];

    // 4: add other quiet moves
    for(unsigned char idx = 0; idx < nNonCaptures; idx++)
        moves[newNum++] = nonCaptures[idx];

    // 5: add losing captures
    while(nCaptures) {
        moves[newNum++] = captures[--nCaptures];
    }

    assert(num == newNum);
}

// ---quiescence search---
// only searching for captures at the end of a regular search in order to ensure the engine won't miss obvious tactics
int quiesce(int alpha, int beta) {
    if(!(nodesQ & 4095) && !infiniteTime) {
        int currTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
        if(currTime >= stopTime) timeOver = true;
    }
    if(timeOver) return 0;
    nodesQ++;

    if(board.isDraw()) return 0;

    int moves[256];
    unsigned char num = board.generateLegalMoves(moves);

    int standPat = evaluate();
    if(standPat >= beta)
        return beta;

    // ---delta pruning---
    // we test if the greatest material swing is enough to raise alpha
    // if it isn't, then the position is hopeless so searching deeper won't improve it
    const int DELTA = 975 + 875 + 200; // capturing a queen + promoting a pawn to a queen + safety margin
    if(DELTA + standPat < alpha) return alpha;

    alpha = max(alpha, standPat);

    sortMoves(moves, num, -1);
    for(unsigned char idx = 0; idx < num; idx++)  {
        if(!isCapture(moves[idx]) && !isPromotion(moves[idx])) continue;

        board.makeMove(moves[idx]);
        int score = -quiesce(-beta, -alpha);
        board.unmakeMove(moves[idx]);

        if(timeOver) return 0;

        if(score >= beta) return beta;

        alpha = max(alpha, score);
    }

    return alpha;
}

// alpha-beta algorithm with a fail-hard framework and PVS
int alphaBeta(int alpha, int beta, short depth, short ply, bool doNull, bool isPV) {
    assert(depth >= 0);

    if(!(nodesSearched & 4095) && !infiniteTime) {
        int currTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
        if(currTime >= stopTime) timeOver = true;
    }
    if(timeOver) return 0;
    nodesSearched++;

    char hashFlag = HASH_F_ALPHA;

    bool isInCheck = board.isInCheck();

    // increase the depth if king is in check
    if(isInCheck) depth++;

    // ---mate distance pruning---
    // if we find mate, we shouldn't look for a better move
    int mateScore = MATE_EVAL-ply;

    if(alpha < -mateScore) alpha = -mateScore;
    if(beta > mateScore) beta = mateScore;
    if(alpha >= beta) return alpha;

    if(board.isDraw()) return 0;

    int moves[256];
    unsigned char num = board.generateLegalMoves(moves);
    if(num == 0) {
        if(isInCheck)
            return -mateScore; //checkmate
        return 0; //stalemate
    }

    // retrieving the hashed move and evaluation if there is any
    int hashScore = probeHash(depth, alpha, beta);
    if(hashScore != VAL_UNKNOWN) {
        // we return hashed info only if it is an exact hit in pv nodes
        if(!isPV || (hashScore > alpha && hashScore < beta))
            return hashScore;
    }

    if(depth == 0) {
        int q = quiesce(alpha, beta);
        if(timeOver) return 0;
        return q;
    }
    int currBestMove = NO_MOVE;

    // ---null move pruning---
    // if our position is good, we can pass the turn to the opponent
    // and if that doesn't wreck our position, we don't need to search further
    const int ENDGAME_MATERIAL = 10;
    if((!isPV) && (isInCheck == false) && ply && (depth >= 3) && (evaluate() >= beta) && doNull && (gamePhase >= ENDGAME_MATERIAL)) {
        board.makeMove(NO_MOVE);

        short R = (depth > 6 ? 3 : 2);
        int score = -alphaBeta(-beta, -beta+1, depth-R-1, ply+1, false, false);

        board.unmakeMove(NO_MOVE);

        if(timeOver) return 0;
        if(score >= beta) return beta;
    }

    // decide if we can apply futility pruning
    bool fPrune = false;
    const int F_MARGIN[4] = { 0, 200, 300, 500 };
    if (depth <= 3 && !isPV && !isInCheck && abs(alpha) < 9000 && evaluate() + F_MARGIN[depth] <= alpha)
        fPrune = true;

    char movesSearched = 0;
    bool raisedAlpha = false;

    sortMoves(moves, num, ply);
    for(unsigned char idx = 0; idx < num; idx++) {
        if(alpha >= beta) return alpha;

        board.makeMove(moves[idx]);

        // ---futility pruning---
        // if a move is bad enough that it wouldn't be able to raise alpha, we just skip it
        // this only applies close to the horizon depth
        if(fPrune && !isCapture(moves[idx]) && !isPromotion(moves[idx]) && !board.isInCheck()) {
            board.unmakeMove(moves[idx]);
            continue;
        }

        int score;

        // ---late move reduction---
        // we do full searches only for the first moves, and then do a reduced search
        // if the move is potentially good, we do a full search instead
        short reductionDepth = 0;
        if(movesSearched > 3 && !isCapture(moves[idx]) && !isPromotion(moves[idx]) && !isInCheck && depth > 4 && !board.isInCheck()) {
            reductionDepth = short(sqrt(double(depth-1)) + sqrt(double(movesSearched-1))); 
            if(isPV) reductionDepth /= 3;
            reductionDepth = (reductionDepth < depth-1 ? reductionDepth : depth-1);

            depth -= reductionDepth;
        } 

            
        // ---principal variation search---
        // we do a full search only until we find a move that raises alpha and we consider it to be the best
        // for the rest of the moves we start with a quick (null window - beta = alpha+1) search
        // and only if the move has potential to be the best, we do a full search
        bool repeatPVSearch;
        do {
            repeatPVSearch = false;

            if(!raisedAlpha) {
                score = -alphaBeta(-beta, -alpha, depth-1, ply+1, true, true);
            } else {
                score = -alphaBeta(-alpha-1, -alpha, depth-1, ply+1, true, false);
                if(score > alpha && score < beta)
                    score = -alphaBeta(-beta, -alpha, depth-1, ply+1, true, true);
            }

            // move can be good, we do a full depth search
            if(reductionDepth && score > alpha) {
                depth += reductionDepth;
                reductionDepth = 0;
                
                repeatPVSearch = true;
            }
        } while(repeatPVSearch);

        board.unmakeMove(moves[idx]);
        movesSearched++;

        if(timeOver) return 0;

        if(score >= beta) {
            recordHash(depth, beta, HASH_F_BETA, currBestMove);

            // killer moves are quiet moves that cause a beta cutoff and are used for sorting purposes
            storeKiller(ply, moves[idx]);

            return beta;
        }

        if(score > alpha) {
            hashFlag = HASH_F_EXACT;
            alpha = score;
            raisedAlpha = true;

            currBestMove = moves[idx];
        }
        
    }

    if(!timeOver && ply == 0) bestMove = currBestMove;
    recordHash(depth, alpha, hashFlag, currBestMove);
    return alpha;
}

const int ASP_INCREASE = 50;
pair<int, int> search() {
    bestMove = NO_MOVE;
    timeOver = false;

    int alpha = -INF, beta = INF;
    int eval = 0;

    for(short i = 0; i < 200; i++)
        killerMoves[i][0] = killerMoves[i][1] = NO_MOVE;

    //---iterative deepening---
    // we start with a depth 1 search and then we increase the depth by 1 every time
    // this helps manage the time because at any point the engine can return the best move found so far
    // also it helps improve move ordering by memorizing the best move that we can search first in the next iteration
    int currStartTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    for(short depth = 1; depth <= maxDepth; ) {
        nodesSearched = nodesQ = 0;

        int curEval = alphaBeta(alpha, beta, depth, 0, false, true);

        if(timeOver) {
            break;
        }

        // ---aspiration window---
        // we start with a full width search (alpha = -inf and beta = inf), modifying them accordingly
        // if we fall outside the current window, we do a full width search with the same depth
        // and if we stay inside the window, we only increase it by a small number, so that we can achieve more cutoffs
        if(curEval <= alpha || curEval >= beta) {
            alpha = -INF;
            beta = INF;
            continue;
        }

        eval = curEval;
        alpha = eval-ASP_INCREASE; // increase window for next iteration
        beta = eval+ASP_INCREASE;

        UCI::showSearchInfo(depth, nodesSearched+nodesQ, currStartTime, eval);
        currStartTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

        depth++; // increase depth only if we are inside the window
    }

    return {retrieveBestMove(), eval};
}
