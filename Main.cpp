#include <bits/stdc++.h>

#include "Search.h"
#include "Evaluate.h"
#include "Board.h"

ifstream fin("pos.txt");
ofstream fout("output.txt");

extern unordered_map<unsigned long long, pair<Move, pair<int, int> > > transpositionTable;

int main() {
    Init();
    initTables();
    generateZobristHashNumbers();

    std::string s, pos;
    while(fin >> s) {
        pos += (s + " ");
    }
    board.LoadFenPos(pos);

    int perspective = (board.turn == White ? 1 : -1);

//    time_t start, end;
//    time(&start);
//    ios_base::sync_with_stdio(false);

    fout << perspective * Search(4, -1000000, 1000000) << '\n';

//    for(int i = 1; i <= 4; i++) {
//        Move m = transpositionTable[board.zobristHash].first;
//        fout << moveToString(m) << ' ';
//        board.makeMove(m);
//    }

//    time(&end);
//    double time_taken = double(end - start);
//    fout << "Time taken by program is : " << fixed
//         << time_taken << setprecision(5);
//    fout << " sec " << endl;
}
