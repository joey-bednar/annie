#include "defs.h"
#include <stdio.h>

#define MATE 50000
#define MATETHRESHOLD 50
#define INF 99999

// return the value of all pieces of a given color using piece values and square
// tables
static int computePieceTotals(ULL bb, int color, BOARD_STATE *board) {
    int total = 0;

    const int val[] = {100, 320, 330, 500, 900, 20000};

    while (bb != 0) {
        int index64 = bitScanForward(bb);
        int sq = SQ64SQ120(index64);

        int piece = getGenericPieceSq120(sq, board);

        ULL mask = (~1ULL << (index64));
        bb &= mask;

        if (color == BLACK) {
            index64 ^= 7;
        }

        switch (piece) {
        case bbPawn:
            total += pawnSqTable[index64];
            break;
        case bbRook:
            total += rookSqTable[index64];
            break;
        case bbBishop:
            total += bishopSqTable[index64];
            break;
        case bbKnight:
            total += knightSqTable[index64];
            break;
        case bbQueen:
            total += queenSqTable[index64];
            break;
        case bbKing:
            total += kingSqTable[index64];
            break;
        }

        total += val[piece - bbPawn];
    }
    return total;
}

int eval(BOARD_STATE *board) {
    // static int count = 0;
    // count++;
    // printf("count: %d\n", count);

    ULL mine = board->bitboard[board->turn];
    ULL yours = board->bitboard[!board->turn];

    int total = 0;

    total += computePieceTotals(mine, board->turn, board);
    total -= computePieceTotals(yours, !board->turn, board);

    return total;
}

// static int quiesce(BOARD_STATE *board, int alpha, int beta) {
//     int stand_pat = eval(board);
//     if (stand_pat >= beta) {
//         return beta;
//     }
//     if (alpha < stand_pat) {
//         alpha = stand_pat;
//     }
//
//     if (board->halfmove >= 100) {
//         return 0;
//     }
//
//     // TODO: add repetition check
//     // TODO: add max depth check
//
//     MOVE moves[MAX_LEGAL_MOVES];
//     int n_moves = generateMoves(board, moves);
//
//     int legal = 0;
//
//     for (int i = 0; i < n_moves; i++) {
//         if (isLegalMove(board, moves[i])) {
//             legal++;
//
//             if (moves[i].type > MOVE_QUEENCASTLE &&
//                 isLegalMove(board, moves[i])) {
//
//                 legal++;
//                 makeMove(board, moves[i]);
//
//                 int score = -quiesce(board, -beta, -alpha);
//                 // int score = eval(board);
//                 unmakeMove(board, moves[i]);
//
//                 if (score >= beta) {
//                     return beta;
//                 }
//                 if (score > alpha) {
//                     alpha = score;
//                 }
//             }
//         }
//     }
//
//     if (legal == 0) {
//         if (isAttacked(board, board->kings[board->turn], !board->turn)) {
//             // checkmate
//             return -MATE; // - depth;
//         } else {
//             // stalemate
//             return 0;
//         }
//     }
//
//     return alpha;
// }

static int alphabeta(BOARD_STATE *board, int depth, int alpha, int beta) {
    int score = -INF;

    int legal = 0;

    if (depth == 0) {
        return eval(board);
    }

    if (board->halfmove >= 100) {
        return 0;
    }

    // TODO: add repetition check
    // TODO: add max depth check

    MOVE moves[MAX_LEGAL_MOVES];
    int n_moves = generateMoves(board, moves);

    for (int i = 0; i < n_moves; i++) {
        if (isLegalMove(board, moves[i])) {

            legal++;
            makeMove(board, moves[i]);

            score = -alphabeta(board, depth - 1, -beta, -alpha);
            unmakeMove(board, moves[i]);

            if (score >= beta) {
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
    }

    if (legal == 0) {
        if (isAttacked(board, board->kings[board->turn], !board->turn)) {
            // checkmate
            return -MATE - depth;
        } else {
            // stalemate
            return 0;
        }
    }

    return alpha;
}

int negaMax(BOARD_STATE *board, int depth) {
    if (depth == 0) {
        return eval(board);
    }
    int max = -INF;

    MOVE moves[MAX_LEGAL_MOVES];
    int n_moves = generateMoves(board, moves);

    for (int i = 0; i < n_moves; i++) {
        if (isLegalMove(board, moves[i])) {

            makeMove(board, moves[i]);

            int score = -negaMax(board, depth - 1);

            if (score > max) {
                max = score;
            }

            unmakeMove(board, moves[i]);
        }
    }

    return max;
}

static void printMoveText(MOVE move) {

    char startFile = SQ120F(SQ64SQ120(START(move.compress))) + 'a';
    char startRank = SQ120R(SQ64SQ120(START(move.compress))) + '1';
    char endFile = SQ120F(SQ64SQ120(END(move.compress))) + 'a';
    char endRank = SQ120R(SQ64SQ120(END(move.compress))) + '1';

    char promote = '\0';
    if (move.type == MOVE_QUEENPROMOTE ||
        move.type == MOVE_QUEENPROMOTECAPTURE) {
        promote = 'q';
    } else if (move.type == MOVE_ROOKPROMOTE ||
               move.type == MOVE_ROOKPROMOTECAPTURE) {
        promote = 'r';
    } else if (move.type == MOVE_BISHOPPROMOTE ||
               move.type == MOVE_BISHOPPROMOTECAPTURE) {
        promote = 'b';
    } else if (move.type == MOVE_KNIGHTPROMOTE ||
               move.type == MOVE_KNIGHTPROMOTECAPTURE) {
        promote = 'k';
    }

    if (promote != '\0') {
        printf("%c%c%c%c%c", startFile, startRank, endFile, endRank, promote);
    } else {
        printf("%c%c%c%c", startFile, startRank, endFile, endRank);
    }
}

static void printInfo(int score, int depth) {
    if (score + MATETHRESHOLD >= MATE && score - MATETHRESHOLD <= MATE) {
        int dist = (MATE - score) + depth;
        int mate = (dist + 1) / 2;
        printf("info score mate %d\n", mate);
    } else if (score + MATETHRESHOLD >= -MATE &&
               score - MATETHRESHOLD <= -MATE) {
        int dist = (-MATE - score) + depth;
        int mate = (dist + 1) / 2;
        printf("info score mate -%d\n", mate);
    } else {
        printf("info score cp %d\n", score);
    }
}

void printBestMove(int depth, BOARD_STATE *board) {
    MOVE best;

    int max = -INF;
    int alpha = -INF;
    int beta = INF;

    MOVE moves[MAX_LEGAL_MOVES];
    int n_moves = generateMoves(board, moves);

    for (int i = 0; i < n_moves; i++) {
        if (isLegalMove(board, moves[i])) {
            makeMove(board, moves[i]);
            int score = -alphabeta(board, depth - 1, -beta, -alpha);
            unmakeMove(board, moves[i]);
            if (score >= max) {
                best = moves[i];
                max = score;

                printInfo(score, depth);

                printf("info currmove ");
                printMoveText(best);
                printf("\n");
            }
        }
    }

    printf("bestmove ");
    printMoveText(best);
    printf("\n");
}
