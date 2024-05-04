# ciorap-bot

This is a UCI-compatible chess engine with ~2100 Elo rating.

### Features

- Hybrid move generation using bitboards and mailbox

####

- Piece evaluation using piece-square tables and mobility bonus
- Pawn structure evaluation that recognizes passed / weak / doubled pawns
- King safety evaluation
- All evaluation parameters tuned using supervised learning

####

- Move searching using alpha-beta algorithm and Principal Variation Search
- Iterative deepening and aspiration windows
- Transposition table using Zobrist hash
- Move ordering using PV-move and MVV-LVA, killer move and history heuristics
- Late move reductions
- Quiescence search with delta-pruning

## Usage

### Compiling and running the engine

```
cd engine
g++ -std=c++20 -O3 *.cpp -o ciorap-bot
./ciorap-bot
```

### Training the NNUE
```
cd nnue

// Create shared C library for parsing the training data
g++ -c -fPIC -static trainDataLoader.cpp -o trainDataLoader.o
g++ -shared -static -o trainDataLoader.dll trainDataLoader.o

// Execute training code
python nnue.py
```
