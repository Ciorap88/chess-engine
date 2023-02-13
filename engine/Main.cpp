#include <unordered_map>

#include "Search.h"
#include "Evaluate.h"
#include "Board.h"
#include "MagicBitboards.h"
#include "TranspositionTable.h"
#include "UCI.h"

int main() {
    init();
    UCI::UCICommunication();
}
