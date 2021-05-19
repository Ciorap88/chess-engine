#include <bits/stdc++.h>

#include "Search.h"
#include "Evaluate.h"
#include "Board.h"

ifstream fin("pos.txt");
ofstream fout("output.txt");

int main() {
    Init();
    initTables();

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

        cout << moveToString(result.first) << ' '<< result.second << '\n';
        board.makeMove(result.first);

        vector<Move> moves = board.GenerateLegalMoves();

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
