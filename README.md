# chess-engine

This is a UCI-compatible chess engine with ~1800 elo rating (just an estimation, I can't know for sure until it will be tested).

Features: 
     - Pseudo-legal and legal move generation using bitboards
     - Position evaluation using independent piece evaluation
     - Minimax algorithm with Alpha-Beta pruning and Principal Variation Search
     - Transposition table using Zobrist hash
     - Other pruning techniques such as null move pruning or futility pruning
     - Quiescence search
     - Move ordering using MVV-LVA and killer move heuristics
     - Iterative deepening and aspiration windows

