#include <bits/stdc++.h>
#include <unistd.h>

#include "Search.h"
#include "Evaluate.h"
#include "Board.h"
#include "MagicBitboards.h"
#include "TranspositionTable.h"
#include "UCI.h"

int main() {
    Init();
    UCI::UCICommunication();
}
