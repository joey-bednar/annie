#include "defs.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int confirmLegalMove(BOARD_STATE *board, MOVE move) {

    MOVE moves[MAX_LEGAL_MOVES];
    int n_moves = generateMoves(board, moves);

    for (int i = 0; i < n_moves; ++i) {
        if (move == moves[i]) {
            makeMove(board, move);
            return !isAttacked(board, SQ64SQ120(getKingSq(board, !board->turn)),
                               board->turn);
        }
    }
    return FALSE;
}

int compareMoves(const void *moveA, const void *moveB) {
    return (CAPTURED(*(MOVE *)moveB) - CAPTURED(*(MOVE *)moveA));
}

static void sortMoves(BOARD_STATE *board, MOVE *moves, int n_moves) {

    if (n_moves <= 1) {
        return;
    }

    MOVE m = pvTT(board->hash);
    if (m != 0ull) {
        for (int i = 0; i < n_moves; i++) {
            if (moves[i] == m) {
                MOVE temp = moves[i];
                moves[i] = moves[0];
                moves[0] = temp;
            }
        }
        qsort(moves + 1, n_moves - 1, sizeof(MOVE), compareMoves);
        return;
    }

    qsort(moves, n_moves, sizeof(MOVE), compareMoves);
}

static int isMateEval(int score) {
    return ((score + MATETHRESHOLD >= MATE && score - MATETHRESHOLD <= MATE) ||
            (score + MATETHRESHOLD >= -MATE && score - MATETHRESHOLD <= -MATE));
}

// prints uci centipawn/mate search info
static void printEval(int score, int depth) {

    if (score + MATETHRESHOLD >= MATE && score - MATETHRESHOLD <= MATE) {
        int mate = MATE - score;
        printf("score mate %d ", mate);
    } else if (score + MATETHRESHOLD >= -MATE &&
               score - MATETHRESHOLD <= -MATE) {
        int mate = MATE + score;
        printf("score mate -%d ", mate);
    } else {
        printf("score cp %d ", score);
    }
}

// prints uci search info
static void printInfo(BOARD_STATE *board, float time, int score, int depth) {

    // print search info
    long nps = floor((1000 * (double)board->nodes / time));

    printf("info ");
    printf("depth %d ", depth);
    printEval(score, 0);
    printf("nodes %ld ", (long)board->nodes);
    printf("nps %ld ", nps);
    printf("time %ld ", (long)time);

    // // skip pv output for search depth 0
    // if (board->pvlength[0] == 0) {
    //     printf("\n");
    //     return;
    // }

    printf("pv ");

    int index = 0;
    MOVE move[MAX_DEPTH];

    BOARD_STATE b;
    memcpy(&b, board, sizeof(BOARD_STATE));

    for (int i = 0; i < depth; i++) {
        move[index] = pvTT(b.hash);
        if (!confirmLegalMove(&b, move[index])) {
            break;
        }
        printMoveText(move[index]);
        printf(" ");
        index++;
    }

    printf("\n");
}

static void checkTime(BOARD_STATE *board) {
    if (board->nodes % 1024 == 0) {
        float new_t = clock() - board->start;
        float time_taken_ms = (1000 * new_t) / CLOCKS_PER_SEC;
        if (time_taken_ms > board->cutoffTime) {
            board->stopped = TRUE;
        }
    }
}

static int quiesce(BOARD_STATE *board, int depth, int alpha, int beta) {
    checkTime(board);
    if (board->stopped) {
        return 0;
    }
    ++board->nodes;

    int score = -INF;

    int legal = 0;

    int stand_pat = eval(board);

    if (board->halfmove >= 100) {
        return 0;
    }

    if (isThreeFold(board)) {
        return 0;
    }

    if (isInsufficientMaterial(board)) {
        return 0;
    }

    if (depth == 0) {
        return stand_pat;
    }

    if (stand_pat >= beta) {
        return beta;
    }
    if (alpha < stand_pat) {
        alpha = stand_pat;
    }

    MOVE moves[MAX_LEGAL_MOVES];
    int n_moves = generateMoves(board, moves);

    // sortMoves(board, moves, n_moves);
    qsort(moves, n_moves, sizeof(MOVE), compareMoves);

    for (int i = 0; i < n_moves; ++i) {

        makeMove(board, moves[i]);
        ++board->ply;

        if (!isAttacked(board, SQ64SQ120(getKingSq(board, !board->turn)),
                        board->turn)) {
            ++legal;

            if (CAPTURED(moves[i]) != EMPTY) {
                score = -quiesce(board, depth - 1, -beta, -alpha);
            }
        }

        unmakeMove(board, moves[i]);
        --board->ply;

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    if (legal == 0) {
        if (isAttacked(board, SQ64SQ120(getKingSq(board, board->turn)),
                       !board->turn)) {
            // checkmate
            return -MATE + board->ply;
        } else {
            // stalemate
            return 0;
        }
    }

    return alpha;
}

static int nullmovesearch(BOARD_STATE *board, int depth, int alpha, int beta) {

    checkTime(board);
    if (board->stopped) {
        return 0;
    }
    ++board->nodes;

    int score = -INF;

    int legal = 0;

    if (board->halfmove >= 100 || isThreeFold(board) ||
        isInsufficientMaterial(board)) {
        return 0;
    }

    // check extension
    int incheck = isAttacked(board, SQ64SQ120(getKingSq(board, board->turn)),
                             !board->turn);
    if (incheck) {
        ++depth;
    }

    if (depth == 0) {
        return quiesce(board, QMAXDEPTH, alpha, beta);
    }

    MOVE moves[MAX_LEGAL_MOVES];
    int n_moves = generateMoves(board, moves);

    qsort(moves, n_moves, sizeof(MOVE), compareMoves);
    // sortMoves(board, moves, n_moves);

    for (int i = 0; i < n_moves; ++i) {

        makeMove(board, moves[i]);
        ++board->ply;

        if (!isAttacked(board, SQ64SQ120(getKingSq(board, !board->turn)),
                        board->turn)) {
            ++legal;
            score = -nullmovesearch(board, depth - 1, -beta, -alpha);
        }

        unmakeMove(board, moves[i]);
        --board->ply;

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    if (legal == 0) {
        if (incheck) {
            // checkmate
            return -MATE + board->ply;
        } else {
            // stalemate
            return 0;
        }
    }

    return alpha;
}

static int alphabeta(BOARD_STATE *board, int depth, int alpha, int beta,
                     int doNull) {

    int hashf = TT_ALPHA_FLAG;

    checkTime(board);
    if (board->stopped) {
        return 0;
    }
    ++board->nodes;

    board->pvlength[board->ply] = board->ply;

    int score = -INF;

    int legal = 0;

    if (board->halfmove >= 100 || isThreeFold(board) ||
        isInsufficientMaterial(board)) {
        return 0;
    }

    // check extension
    int incheck = isAttacked(board, SQ64SQ120(getKingSq(board, board->turn)),
                             !board->turn);
    if (incheck) {
        ++depth;
    }

    // MOVE bestmove;
    // int probe = probeTT(board->hash, &bestmove, alpha, beta, depth);
    // if (probe != TT_EMPTY && board->ply > 0) {
    //     return probe;
    // }

    if (depth == 0) {
        int score = quiesce(board, QMAXDEPTH, alpha, beta);
        // storeTT(board->hash, 0ull, score, TT_EXACT_FLAG, depth);
        return score;
    }

    if (depth >= 4 && !incheck) {

        makeNullMove(board);
        int nullScore = -nullmovesearch(board, depth - 2, -beta, -beta + 1);
        unmakeNullMove(board);

        if (nullScore >= beta) {
            return beta;
        }
    }

    MOVE moves[MAX_LEGAL_MOVES];
    int n_moves = generateMoves(board, moves);

    // qsort(moves, n_moves, sizeof(MOVE), compareMoves);
    sortMoves(board, moves, n_moves);

    for (int i = 0; i < n_moves; ++i) {

        makeMove(board, moves[i]);
        ++board->ply;

        if (!isAttacked(board, SQ64SQ120(getKingSq(board, !board->turn)),
                        board->turn)) {
            ++legal;
            score = -alphabeta(board, depth - 1, -beta, -alpha, doNull);
        }

        unmakeMove(board, moves[i]);
        --board->ply;

        if (score >= beta) {
            storeTT(board->hash, moves[i], beta, TT_BETA_FLAG, depth);
            return beta;
        }
        if (score > alpha) {
            alpha = score;

            hashf = TT_EXACT_FLAG;
            // storeTT(board->hash, moves[i], score, hashf, depth);
            storePVTT(board->hash, moves[i], score, hashf, depth);

            // // add moves to pvarray
            // board->pvarray[board->ply][board->ply] = moves[i];
            // for (int j = board->ply + 1; j < board->pvlength[board->ply + 1];
            //      ++j) {
            //     board->pvarray[board->ply][j] =
            //         board->pvarray[board->ply + 1][j];
            // }
            // board->pvlength[board->ply] = board->pvlength[board->ply + 1];
        }
    }

    if (legal == 0) {
        if (incheck) {
            // checkmate
            return -MATE + board->ply;
        } else {
            // stalemate
            return 0;
        }
    }

    // storeTT(board->hash, 0ull, score, hashf, depth);
    return alpha;
}

void printMoveText(MOVE move) {

    char startFile = SQ120F(START120(move)) + 'a';
    char startRank = SQ120R(START120(move)) + '1';
    char endFile = SQ120F(END120(move)) + 'a';
    char endRank = SQ120R(END120(move)) + '1';

    char promote = '\0';
    if (PROMOTED(move) == bbQueen) {
        promote = 'q';
    } else if (PROMOTED(move) == bbRook) {
        promote = 'r';
    } else if (PROMOTED(move) == bbBishop) {
        promote = 'b';
    } else if (PROMOTED(move) == bbKnight) {
        promote = 'n';
    }

    if (promote != '\0') {
        printf("%c%c%c%c%c", startFile, startRank, endFile, endRank, promote);
    } else {
        printf("%c%c%c%c", startFile, startRank, endFile, endRank);
    }
}

// returns true when next iteration of search should be
// prevented due to time restrictions
static int searchCutoff(BOARD_STATE *board, float time_ms) {

    int time = inputTime[board->turn];
    if (time == DEFAULT_TIME) {
        return FALSE;
    }

    // emergency
    if (time <= 1000 * 5 && time_ms > 100) {
        return TRUE;
    }

    // bullet time control
    if (time <= 1000 * 60 * 1 && time_ms > 300) {
        return TRUE;
    }

    // blitz
    if (time <= 1000 * 60 * 3 && time_ms > 900) {
        return TRUE;
    }
    if (time <= 1000 * 60 * 5 && time_ms > 1100) {
        return TRUE;
    }
    if (time <= 1000 * 60 * 7 && time_ms > 1500) {
        return TRUE;
    }

    // rapid
    if (time_ms >= 5000 * 1) {
        return TRUE;
    }
    return FALSE;
}

static float setCutoff(BOARD_STATE *board) {
    int time = inputTime[board->turn];
    int inc = inputInc[board->turn];

    if (time == DEFAULT_TIME) {
        return 1000 * 60 * 36;
    }

    // emergency
    if (time <= 1000 * 5) {
        return 150 + inc;
    }

    // bullet
    if (time <= 1000 * 60) {
        return 5000 + inc;
    }

    // blitz
    if (time <= 1000 * 60 * 3) {
        return 8000 + inc;
    }

    // blitz
    if (time <= 1000 * 60 * 5) {
        return 10000 + inc;
    }

    // rapid
    return 20000 + inc;
}

void search(BOARD_STATE *board) {

    // reset nodes searched
    board->nodes = 0;

    // reset search time parameters
    board->stopped = FALSE;
    board->cutoffTime = setCutoff(board);

    // begin timer
    board->start = clock();

    // iterative deepening
    MOVE bestmove;
    int alpha = -INF;
    int beta = INF;
    for (int searchDepth = 1; searchDepth <= inputDepth; searchDepth++) {

        board->ply = 0;

        int score = alphabeta(board, searchDepth, alpha, beta, TRUE);

        if (board->stopped) {
            break;
        }

        // // redo search with INF window if score is outside aspiration window
        // if (score <= alpha || score >= beta) {
        //     alpha = -INF;
        //     beta = INF;
        //     searchDepth--;
        //     continue;
        // }
        //
        // // set aspiration window
        // alpha = score - ASPIRATION_WINDOW;
        // beta = score + ASPIRATION_WINDOW;

        // measure time spent
        float new_t = clock() - board->start;
        float time_taken_ms = (1000 * new_t) / CLOCKS_PER_SEC;

        // print search info
        printInfo(board, time_taken_ms, score, searchDepth);

        // get bestmove/ponder from pv
        bestmove = pvTT(board->hash);

        // end searches in timed games if mate is found
        if (isMateEval(score) && inputTime[board->turn] != DEFAULT_TIME) {
            break;
        }

        // prevent new searches if not enough time
        if (searchCutoff(board, time_taken_ms)) {
            break;
        }

        // printBoard(board);
    }

    // print best move
    printf("bestmove ");
    printMoveText(bestmove);
    printf("\n");
}
