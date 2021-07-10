#include <bits/stdc++.h>
#include <unistd.h>

#include "Search.h"
#include "Evaluate.h"
#include "Board.h"

ifstream fin("pos.txt");
ofstream fout("output.txt");

int main() {
    Init();

    std::string s, pos;
    while(fin >> s) {
        pos += (s + " ");
    }
    board.LoadFenPos(pos);
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
