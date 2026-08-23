<div align="center">

# Amelia
#### Chess engine accepting challenges on [Lichess.org](https://lichess.org/@/AmeliaLatest).

[![status](https://lichess-badges.vercel.app/api/status/AmeliaLatest)](https://lichess.org/@/AmeliaLatest/)

[![bullet](https://lichess-badges.vercel.app/api/rating/AmeliaLatest/bullet)](https://lichess.org/@/AmeliaLatest/perf/bullet)
[![blitz](https://lichess-badges.vercel.app/api/rating/AmeliaLatest/blitz)](https://lichess.org/@/AmeliaLatest/perf/blitz)
[![rapid](https://lichess-badges.vercel.app/api/rating/AmeliaLatest/rapid)](https://lichess.org/@/AmeliaLatest/perf/rapid)

<img src="logo.png" alt="logo" width="200px" height="200px"/>

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)

</div>

## :hibiscus: About

Amelia is a [UCI](https://www.chessprogramming.org/UCI) compatible chess engine written
from scratch in C. You can play Amelia on [Lichess.org](https://lichess.org/@/AmeliaLatest) or
install a chess interface to play locally.

## :hibiscus: Lichess

Challenge Amelia on Lichess under the username [AmeliaLatest](https://lichess.org/@/AmeliaLatest).
You can also [spectate active games](https://lichess.org/@/AmeliaLatest/tv).

## :hibiscus: Technical Features

- Board
    - [Bitboards](https://www.chessprogramming.org/Bitboards)
    - [Zobrist hashing](https://en.wikipedia.org/wiki/Zobrist_hashing)
- Search
    - [Principal Variation Search](https://en.wikipedia.org/wiki/Principal_variation_search)
    - [Quiescence search](https://en.wikipedia.org/wiki/Quiescence_search)
    - [Iterative deepening](https://www.chessprogramming.org/Iterative_Deepening)
    - [Transposition table](https://en.wikipedia.org/wiki/Transposition_table)
    - Move ordering ([hash move](https://www.chessprogramming.org/Hash_Move), [MVV-LVA](https://www.chessprogramming.org/MVV-LVA), [killer moves](https://www.chessprogramming.org/Killer_Move), [history heuristic](https://www.chessprogramming.org/History_Heuristic))
    - [Check extensions](https://www.chessprogramming.org/Check_Extensions)
    - [Killer heuristic](https://www.chessprogramming.org/Killer_Heuristic)
    - [History heuristic](https://www.chessprogramming.org/History_Heuristic)
    - [Null move pruning](https://en.wikipedia.org/wiki/Null-move_heuristic)
    - [Late move reductions](https://en.wikipedia.org/wiki/Late_move_reductions)
- Evaluation
    - [Tapered](https://www.chessprogramming.org/Tapered_Eval) middle/endgame evaluation
    - [Piece-square tables](https://www.chessprogramming.org/Piece-Square_Tables)
    - [Mop-up evaluation](https://www.chessprogramming.org/Mop-up_Evaluation)
    - Passed pawn bonus
    - Bishop pair bonus
- Interface
    - [UCI](https://en.wikipedia.org/wiki/Universal_Chess_Interface) compatible
    - [Pondering](https://www.chessprogramming.org/Pondering)
    - Bulk [perft](https://www.chessprogramming.org/Perft) testing

## :hibiscus: Build

```bash
git clone https://github.com/joey-bednar/amelia && \
cd amelia && \
git checkout <VERSION> && \
make && \
./build/amelia
```

Optionally, you may cross-compile for Windows using `make TARGET=Windows` or use any [GCC `-march` option](https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html).
For example, `make ARCH=native` optimizes for your system.
