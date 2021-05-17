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

    int perspective = (board.turn == White ? 1 : -1);
    fout << perspective * Search(6, -1000000, 1000000) << '\n';
}
