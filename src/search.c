#include "defs.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int compareMoves(const void *moveA, const void *moveB) {

    MOVE A = *(MOVE *)moveA;
    MOVE B = *(MOVE *)moveB;

    return MVVLVA(PIECE(B), CAPTURED(B)) - MVVLVA(PIECE(A), CAPTURED(A));
}

static void sortMoves(BOARD_STATE *board, MOVE *moves, int n_moves) {

    if (n_moves <= 1) {
        return;
    }

    MOVE best;
    if (probeTT(board->hash, &best, INF, -INF, 0) != TT_EMPTY) {
        for (int i = 0; i < n_moves; i++) {
            if (moves[i] == best) {
                MOVE temp = moves[i];
                moves[i] = moves[0];
                moves[0] = temp;
                break;
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
        int mate = (depth + 1) / 2;
        printf("score mate %d ", mate);
    } else if (score + MATETHRESHOLD >= -MATE &&
               score - MATETHRESHOLD <= -MATE) {
        int mate = (depth) / 2;
        printf("score mate -%d ", mate);
    } else {
        printf("score cp %d ", score);
    }
}

static void printPV(BOARD_STATE *board, int depth) {
    BOARD_STATE bcopy;
    memcpy(&bcopy, board, sizeof(BOARD_STATE));

    for (int i = 0; i < depth; ++i) {
        MOVE m;
        if (probeTT(bcopy.hash, &m, INF, -INF, 0) == TT_EMPTY) {
            return;
        }
        printMoveText(m);
        printf(" ");
        makeMove(&bcopy, m);
    }
}

// prints uci search info
static void printInfo(BOARD_STATE *board, float time, int score, int depth) {

    // print search info
    long nps = floor((1000 * (double)board->nodes / time));

    printf("info ");
    printf("depth %d ", depth);
    MOVE m;
    int val = probeTT(board->hash, &m, INF, -INF, 0);
    printEval(score, val);
    printf("nodes %ld ", (long)board->nodes);
    printf("nps %ld ", nps);
    printf("time %ld ", (long)time);

    // skip pv output for search depth 0
    // if (board->pvlength[0] == 0) {
    //     printf("\n");
    //     return;
    // }

    printf("pv ");
    printPV(board, depth);

    // for (int i = 0; i < board->pvlength[0]; ++i) {
    //     printMoveText(board->pvarray[0][i]);
    //     printf(" ");
    // }
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

            if (CAPTURED(moves[i]) != EMPTY || PROMOTED(moves[i]) != EMPTY) {
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

    // qsort(moves, n_moves, sizeof(MOVE), compareMoves);
    sortMoves(board, moves, n_moves);

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
            storeTT(board->hash, moves[i], beta, TT_BETA_FLAG, depth);
            return beta;
        }
        if (score > alpha) {
            storeTT(board->hash, moves[i], score, TT_EXACT_FLAG, depth);
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

    checkTime(board);
    if (board->stopped) {
        return 0;
    }
    ++board->nodes;

    MOVE bestmove = 0ull;
    int bestscore = -INF;
    int oldalpha = alpha;

    int flag = TT_ALPHA_FLAG;

    int legal = 0;

    if (board->halfmove >= 100 || isThreeFold(board) ||
        isInsufficientMaterial(board)) {
        // storeTT(board->hash, 0ull, 0, TT_EXACT_FLAG, depth);
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

    int val = probeTT(board->hash, &bestmove, alpha, beta, depth);
    if (beta - alpha > 1 && val != TT_EMPTY) {
        return val;
    }

    if (depth >= 4 && !incheck) {

        makeNullMove(board);
        // int nullScore = -alphabeta(board, depth - 2, -beta, -beta + 1,
        // FALSE);
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

    int score = -INF;
    for (int i = 0; i < n_moves; ++i) {

        makeMove(board, moves[i]);
        ++board->ply;

        if (!isAttacked(board, SQ64SQ120(getKingSq(board, !board->turn)),
                        board->turn)) {
            ++legal;

            if (i == 0) {
                // full window search on pv node
                score = -alphabeta(board, depth - 1, -beta, -alpha, doNull);
            } else {
                // check if other moves score better than PV
                score =
                    -alphabeta(board, depth - 1, -alpha - 1, -alpha, doNull);

                // prevent re-search of non-PV nodes
                if (score > alpha && beta - alpha > 1) {
                    score = -alphabeta(board, depth - 1, -beta, -alpha, doNull);
                }
            }
        }

        unmakeMove(board, moves[i]);
        --board->ply;

        if (score > bestscore) {
            bestscore = score;
            bestmove = moves[i];
            if (score > alpha) {
                if (score >= beta) {
                    bestmove = moves[i];
                    storeTT(board->hash, moves[i], beta, TT_BETA_FLAG, depth);
                    return beta;
                }
            }
            flag = TT_EXACT_FLAG;
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

    if (alpha != oldalpha) {
        storeTT(board->hash, bestmove, bestscore, TT_EXACT_FLAG, depth);
    } else {
        storeTT(board->hash, bestmove, alpha, TT_ALPHA_FLAG, depth);
    }

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

// returns TRUE if only one move is legal
// adds only move to "move"
static int onlyMove(BOARD_STATE *board, MOVE *move) {
    MOVE moves[MAX_LEGAL_MOVES];
    int n_moves = generateMoves(board, moves);

    int legal = 0;
    int legalindex = 0;

    int i = 0;
    while (i < n_moves && legal < 2) {
        // for (int i = 0; i < n_moves; ++i) {

        makeMove(board, moves[i]);

        if (!isAttacked(board, SQ64SQ120(getKingSq(board, !board->turn)),
                        board->turn)) {
            ++legal;
            legalindex = i;
        }

        unmakeMove(board, moves[i]);
        i++;
    }

    if (legal == 1) {
        *move = moves[legalindex];
        return TRUE;
    }
    return FALSE;
}

void search(BOARD_STATE *board) {

    MOVE bestmove;

    // reset nodes searched
    board->nodes = 0;

    // reset search time parameters
    board->stopped = FALSE;
    board->cutoffTime = setCutoff(board);

    // begin timer
    board->start = clock();

    // if only one legal move, play instantly
    if (onlyMove(board, &bestmove)) {
        // print best move
        printf("bestmove ");
        printMoveText(bestmove);
        printf("\n");
        return;
    }

    // iterative deepening
    int alpha = -INF;
    int beta = INF;
    for (int searchDepth = 1; searchDepth <= inputDepth; searchDepth++) {

        board->ply = 0;

        int score = alphabeta(board, searchDepth, alpha, beta, TRUE);

        if (board->stopped) {
            break;
        }

        // // redo search with INF window if score is outside aspiration
        // window if (score <= alpha || score >= beta) {
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
        // bestmove = board->pvarray[0][0];
        probeTT(board->hash, &bestmove, INF, -INF, 0);

        // end searches in timed games if mate is found
        if (isMateEval(score) && inputTime[board->turn] != DEFAULT_TIME) {
            break;
        }

        // prevent new searches if not enough time
        if (searchCutoff(board, time_taken_ms)) {
            break;
        }
    }

    // print best move
    printf("bestmove ");
    printMoveText(bestmove);
    printf("\n");

    // initTT();
}
