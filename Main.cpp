#include <bits/stdc++.h>
#include <unistd.h>

#include "Search.h"
#include "Evaluate.h"
#include "Board.h"
#include "MagicBitboards.h"

ifstream depthFile("depth.txt");
ifstream fin("pos.txt");
ofstream fout("output.txt");

int main() {
    Init();

    string pieces, castles, epTargetSq;
    char turn;
    int halfMoveClock = 0, fullMoveNumber = 0;
    fin >> pieces >> turn >> castles >> epTargetSq >> halfMoveClock >> fullMoveNumber;
    board.loadFenPos(pieces, turn, castles, epTargetSq, halfMoveClock, fullMoveNumber);

    while(true) {
        auto result = Search();
        if(result.first.from == -1) {
            cout << "Game over\n";
            break;
        }

        cout << moveToString(result.first) << ' ';
        if(result.second >= MATE_THRESHOLD) cout << "MATE\n";
        else if(result.second <= -MATE_THRESHOLD) cout << "-MATE\n";
        else cout << (board.turn == White ? 1 : -1) * result.second << '\n';

        board.makeMove(result.first);

        vector<Move> moves = board.GenerateLegalMoves();

        if(moves.size() == 0) {
            sleep(3);
            break;
        }

        bool good;
        do {
            cout << "Your Move: ";
            string m;
            cin >> m;
            good = false;
            moves = board.GenerateLegalMoves();
            for(Move mv: moves) {
                if(moveToString(mv) == m) {
                    board.makeMove(mv);
                    good = true;
                    break;
                }
            }
        } while(good == false);
    }
}
