#include <bits/stdc++.h>

#include "Evaluate.h"
#include "Board.h"
#include "Search.h"
#include "TranspositionTable.h"
#include "MagicBitboards.h"
#include "UCI.h"

using namespace std;

unordered_map<U64, int> repetitionMap;
Move killerMoves[200][2];

const int inf = 1000000;
const Move noMove = {-1,-1,0,0,0,0};

Move bestMove = noMove;

const int mateEval = inf-1;
const int MATE_THRESHOLD = mateEval/2;

int startTime, stopTime;
int maxDepth = 200;
bool infiniteTime;

int nodesSearched = 0;
int nodesQ = 0;
bool timeOver = false;

bool areEqual(Move a, Move b) {
    return ((a.from == b.from) && (a.to == b.to) && (a.capture == b.capture) &&
     (a.ep == b.ep) && (a.prom == b.prom) && (a.castle == b.castle));
}

// store unique killer moves
void storeKiller(int ply, Move m) {
    if(m.capture || m.ep) return; // killer moves are by definition quiet moves

    // make sure the moves are different
    if(!areEqual(killerMoves[ply][0], m)) 
        killerMoves[ply][1] = killerMoves[ply][0];

    killerMoves[ply][0] = m;
}

// evaluating moves by material gain
int captureScore(Move m) {
    int score = 0;

    int otherColor = (board.turn ^ (Black | White));

    // captured piece value - capturing piece value
    if(m.capture) score += (pieceValues[(m.capture ^ otherColor)]-
                  pieceValues[(board.squares[m.from] ^ board.turn)]);

    // material gained by promotion
    if(m.prom) score += pieceValues[m.prom] - pieceValues[Pawn];

    return score;
}

bool cmpCapturesDesc(Move a, Move b) {
    return (captureScore(a) < captureScore(b));
}

bool cmpCapturesAsc(Move a, Move b) {
    return  (captureScore(a) > captureScore(b));
}

// ---move ordering---
void sortMoves(vector<Move> &moves, int ply) {
    // sorting in quiescence search
      if(ply == -1) {
          sort(moves.begin(), moves.end(), cmpCapturesAsc);
          return;
      }

    vector<Move> sorted, captures, nonCaptures;

    // find pv move
    Move pvMove = noMove;
    if(ply == 0) pvMove = bestMove;
    if(areEqual(pvMove, noMove)) pvMove = retrieveBestMove();

    // check legality of killer moves
    bool killerLegal[2] = {false, false};
    for(int i = 0; i < 2; i++)
        for(auto m: moves)
            if(areEqual(killerMoves[ply][i], m))
                killerLegal[i] = true;

    // split the other moves into captures and non captures for easier sorting
    for(Move m: moves) {
        if(areEqual(m, pvMove) || areEqual(m, killerMoves[ply][0]) || areEqual(m, killerMoves[ply][1]))
            continue;

        if(m.capture) captures.push_back(m);
        else nonCaptures.push_back(m);
    }

    // 1: add pv move
    if(!areEqual(pvMove, noMove)) sorted.push_back(pvMove);
    
    // 2: add captures sorted by MVV-LVA (only winning / equal captures first)
    sort(captures.begin(), captures.end(), cmpCapturesDesc);
    while(captures.size() && captureScore(captures.back()) >= 0) {
        sorted.push_back(captures.back());
        captures.pop_back();
    }

    // 3: add killer moves
    for(int i = 0; i < 2; i++)
        if(killerLegal[i] && !areEqual(killerMoves[ply][i], noMove) && !areEqual(killerMoves[ply][i], pvMove)) 
            sorted.push_back(killerMoves[ply][i]);

    // 4: add other quiet moves
    for(Move m: nonCaptures)
        sorted.push_back(m);

    // 5: add losing captures
    while(captures.size()) {
        sorted.push_back(captures.back());
        captures.pop_back();
    }

    assert(moves.size() == sorted.size());

    moves = sorted;
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

    vector<Move> moves = board.GenerateLegalMoves();

    int standPat = Evaluate();
    if(standPat >= beta)
        return beta;

    // ---delta pruning---
    // we test if the greatest material swing is enough to raise alpha
    // if it isn't, then the position is hopeless so searching deeper won't improve it
    int delta = 975 + 875 + 200; // capturing a queen + promoting a pawn to a queen + safety margin
    if(delta + standPat < alpha) return alpha;

    alpha = max(alpha, standPat);

    Move best = noMove;

    sortMoves(moves, -1);
    for(Move m : moves)  {
        if(!m.capture && !m.prom) continue;

        board.makeMove(m);
        int score = -quiesce(-beta, -alpha);
        board.unmakeMove(m);

        if(timeOver) return 0;

        if(score >= beta) {
            RecordHash(0, beta, hashFBeta, m);
            return beta;
        }

        if(score > alpha) {
            alpha = score;
            best = m;
        }
    }

    if(!areEqual(best, noMove))
        RecordHash(0, alpha, hashFExact, best);

    return alpha;
}

// alpha-beta algorithm with a lot of enhancements
int alphaBeta(int alpha, int beta, int depth, int ply, bool doNull, bool isPV) {
    assert(depth >= 0);

    if(!(nodesSearched & 4095) && !infiniteTime) {
        int currTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
        if(currTime >= stopTime) timeOver = true;
    }
    if(timeOver) return 0;
    nodesSearched++;

    int hashFlag = hashFAlpha;

    bool isInCheck = board.isInCheck();

    // increase the depth if king is in check
    if(isInCheck) depth++;

    // ---mate distance pruning---
    // if we find mate, we shouldn't look for a better move
    int mateScore = mateEval-ply;

    if(alpha < -mateScore) alpha = -mateScore;
    if(beta > mateScore) beta = mateScore;
    if(alpha >= beta) return alpha;

    if(board.isDraw()) return 0;
    vector<Move> moves = board.GenerateLegalMoves();
    if(moves.size() == 0) {
        if(isInCheck)
            return -mateScore; //checkmate
        return 0; //stalemate
    }

    // retrieving the hashed move and evaluation if there is any
    int hashScore = ProbeHash(depth, alpha, beta);
    if(hashScore != valUnknown) {
        // we return hashed info only if it is an exact hit in pv nodes
        if(!isPV || (hashScore > alpha && hashScore < beta))
            return hashScore;
    }

    if(depth == 0) {
        int q = quiesce(alpha, beta);
        if(timeOver) return 0;
        return q;
    }
    Move currBestMove = noMove;

    // ---null move pruning---
    // if our position is good, we can pass the turn to the opponent
    // and if that doesn't wreck our position, we don't need to search further
    if((!isPV) && (isInCheck == false) && ply && (depth >= 3) && (Evaluate() >= beta) && doNull && (gamePhase >= endgameMaterial)) {
        board.makeMove(noMove);

        int R = (depth > 6 ? 3 : 2);
        int score = -alphaBeta(-beta, -beta+1, depth-R-1, ply+1, false, false);

        board.unmakeMove(noMove);

        if(timeOver) return 0;
        if(score >= beta) return beta;
    }

    // decide if we can apply futility pruning
    bool fPrune = false;
    int fMargin[4] = { 0, 200, 300, 500 };
    if (depth <= 3 && !isPV && !isInCheck && abs(alpha) < 9000 && Evaluate() + fMargin[depth] <= alpha)
        fPrune = true;

    int movesSearched = 0;
    bool raisedAlpha = false;

    sortMoves(moves, ply);
    for(Move m: moves) {
        if(alpha >= beta) return alpha;

        board.makeMove(m);

        // ---futility pruning---
        // if a move is bad enough that it wouldn't be able to raise alpha, we just skip it
        // this only applies close to the horizon depth
        if(fPrune && !m.capture && !m.prom && !board.isInCheck()) {
            board.unmakeMove(m);
            continue;
        }

        int score;

        // ---late move reduction---
        // we do full searches only for the first moves, and then do a reduced search
        // if the move is potentially good, we do a full search instead
        int reductionDepth = 0;
        if(movesSearched > 3 && !m.capture && !m.prom && !isInCheck && depth > 4 && !board.isInCheck()) {
            reductionDepth = int(sqrt(double(depth-1)) + sqrt(double(movesSearched-1))); 
            if(isPV) reductionDepth /= 3;
            reductionDepth = min(reductionDepth, depth-1);

            depth -= reductionDepth;
        } 

    pvSearch:
        // ---principal variation search---
        // we do a full search only until we find a move that raises alpha and we consider it to be the best
        // for the rest of the moves we start with a quick (null window - beta = alpha+1) search
        // and only if the move has potential to be the best, we do a full search
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

            goto pvSearch;
        }

        board.unmakeMove(m);
        movesSearched++;

        if(timeOver) return 0;

        if(score >= beta) {
            RecordHash(depth, beta, hashFBeta, currBestMove);

            // killer moves are quiet moves that cause a beta cutoff and are used for sorting purposes
            storeKiller(ply, m);

            return beta;
        }

        if(score > alpha) {
            hashFlag = hashFExact;
            alpha = score;
            raisedAlpha = true;

            currBestMove = m;
        }
        
    }

    if(!timeOver && ply == 0) bestMove = currBestMove;
    RecordHash(depth, alpha, hashFlag, currBestMove);
    return alpha;
}

const int aspIncrease = 50;
pair<Move, int> Search() {
    bestMove = noMove;
    timeOver = false;

    int alpha = -inf, beta = inf;
    int eval = 0;

    for(int i = 0; i < 200; i++)
        killerMoves[i][0] = killerMoves[i][1] = noMove;

    //---iterative deepening---
    // we start with a depth 1 search and then we increase the depth by 1 every time
    // this helps manage the time because at any point the engine can return the best move found so far
    // also it helps improve move ordering by memorizing the best move that we can search first in the next iteration
    int currStartTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    for(int depth = 1; depth <= maxDepth; ) {
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
            alpha = -inf;
            beta = inf;
            continue;
        }

        eval = curEval;
        alpha = eval-aspIncrease; // increase window for next iteration
        beta = eval+aspIncrease;

        UCI::showSearchInfo(depth, nodesSearched+nodesQ, currStartTime, eval);
        currStartTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

        depth++; // increase depth only if we are inside the window
    }

    return {retrieveBestMove(), eval};
}
