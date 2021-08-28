#include <bits/stdc++.h>

#include "Board.h"
#include "Search.h"
#include "TranspositionTable.h"
#include "UCI.h"

using namespace std;

// function to split string into words
vector<string> splitStr(string s) {
    vector<string> ans;
    for(auto c: s) {
        if(c == ' ' || !ans.size()) ans.push_back("");
        else ans.back() += c;
    }
    return ans;
}

string UCI::engineName = "chess-engine v1";

void UCI::UCICommunication() {
    while(true) {
        string inputString;
        getline(cin, inputString);

        if(inputString == "uci") {
            inputUCI();
        } else if(inputString == "isready") {
            inputIsReady();
        } else if(inputString == "ucinewgame") {
            inputUCINewGame();
        } else if(inputString.substr(0, 8) == "position") {
            inputPosition(inputString);
        } else if(inputString.substr(0, 2) == "go") {
            inputGo(inputString);
        } else if(inputString == "stop") {
            // timeOver = true?
        } else if(inputString == "quit") {
            break;
        }
    }
}

void UCI::inputUCI() {
    cout << "id name " << engineName << '\n';
    cout << "id author Vlad Ciocoiu\n";
    cout << "uciok\n";
}

void UCI::inputIsReady() {
    cout << "readyok\n";
}

// prepare for a new game by clearing hash tables
void UCI::inputUCINewGame() {
    clearTT();
    repetitionMap.clear();
}

void UCI::inputPosition(string input) {

    // split input into words
    vector<string> parsedInput = splitStr(input);
    int movesIdx = 0;

    // load the position
    if(parsedInput[1] == "startpos") {
        board.loadFenPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        movesIdx = 2;
    } else {
        string fen;
        for(int i = 2; i <= 7; i++)
            fen += parsedInput[i] + " ";
        board.loadFenPos(fen);
        movesIdx = 8;
    }

    // make the moves
    for(int i = movesIdx+1; i < parsedInput.size(); i++) {
        vector<Move> moves = board.GenerateLegalMoves();
        for(Move m: moves) 
            if(moveToString(m) == parsedInput[i]) {
                board.makeMove(m);
                break;
            }
    }
}

void UCI::inputGo(string input) {
    int time = -1, inc = 0, depth = 200, movesToGo = 30, moveTime = -1;
    infiniteTime = true;

    vector<string> parsedInput = splitStr(input);

    // loop through words
    for(int i = 0; i < parsedInput.size(); i++) {

        // get time and increment according to our color 
        if(parsedInput[i] == "wtime" && board.turn == White) {
            time = stoi(parsedInput[i+1]);
        }
        if(parsedInput[i] == "btime" && board.turn == Black) {
            time = stoi(parsedInput[i+1]);
        }
        if(parsedInput[i] == "winc" && board.turn == White) {
            inc = stoi(parsedInput[i+1]);
        }
        if(parsedInput[i] == "binc" && board.turn == Black) {
            inc = stoi(parsedInput[i+1]);
        }

        // get moves to go
        if(parsedInput[i] == "movestogo") {
            movesToGo = stoi(parsedInput[i+1]);
        }
        
        // get move time if any
        if(parsedInput[i] == "movetime") {
            moveTime = stoi(parsedInput[i+1]);
        }

        // get max depth
        if(parsedInput[i] == "depth") {
            depth = stoi(parsedInput[i+1]);
        }
    }
    
    // if move time is set, we can use it all for the current move
    if(moveTime != -1) {
        time = moveTime;
        movesToGo = 1;
    }

    // get current time;
    startTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

    // if depth is specified, we change the max depth, otherwise we leave it at 200
    maxDepth = depth;

    // set the stop time
    if(time != -1) {
        infiniteTime = false;
        time /= movesToGo;
        stopTime = startTime + time + inc - 50;
    }

    auto result = Search();
    cout << "bestmove " << moveToString(result.first) << '\n';
}