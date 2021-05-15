#include <bits/stdc++.h>

using namespace std;

const vector<int> pieceValues = {0,100,300,300,500,900,100000};

int countMaterial(int color) {
    int bishops = 0, centralPawns = 0;

    int material = 0;
    for(int i = 0; i < 64; i++) {
        if(board.squares[i] & color) {
            int File = i%8;
            int piece = (board.squares[i] ^ color);

            if(piece == Bishop) bishops++;
            if(piece == Pawn && (File == 3 || File == 4))
                centralPawns++;

            material += (pieceValues[board.squares[i] ^ color]);
        }
    }
    // bonus for bishop pair
    if(bishops >= 2) material += 50;

    // bonus for central pawns
    material += centralPawns * 20;

    return material;
}

int Evaluate() {

    // material
    int materialScore = countMaterial(White) - countMaterial(Black);

    // king safety

    // piece activity

    // pawn structure

    int totalScore = materialScore;

    return totalScore;
}
