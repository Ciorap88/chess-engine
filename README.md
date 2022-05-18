# ciorap-bot

This is a UCI-compatible chess engine with ~2000 Elo rating (only tested against [Snowy](https://github.com/JasonCreighton/snowy) so far).

### Features

- Hybrid move generation using bitboards and mailbox
####
- Piece evaluation using piece-square tables and mobility bonus
- Pawn structure evaluation that recognizes passed / weak / doubled pawns
- King safety evaluation
#### 
- Move searching using alpha-beta algorithm and Principal Variation Search
- Iterative deepening and aspiration windows
- Transposition table using Zobrist hash
- Move ordering using PV-move and MVV-LVA, killer move and history heuristics
- Null move pruning, Late move reductions, futility pruning
- Quiescence search with delta-pruning
