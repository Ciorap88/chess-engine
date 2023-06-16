#include <unordered_map>
#include <thread>

#include "Search.h"
#include "Evaluate.h"
#include "Board.h"
#include "MagicBitboards.h"
#include "TranspositionTable.h"
#include "UCI.h"

int main() {
    init();

    std::thread communicationThread(UCI::UCICommunication);
    std::thread searchThread(UCI::inputGo);

    communicationThread.join();       
    searchThread.join(); 

    return 0;
}
