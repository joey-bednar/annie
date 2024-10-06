#include "defs.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int compareMoves(const void *moveA, const void *moveB) {

    MOVE A = *(MOVE *)moveA;
    MOVE B = *(MOVE *)moveB;

    return MVVLVA(PIECE(B), CAPTURED(B)) - MVVLVA(PIECE(A), CAPTURED(A));
}

static void scoreMoves(BOARD_STATE *board, MOVE *moves, int *scores,
                       int n_moves) {
    MOVE m;
    probeTT(board->hash, &m, INF, -INF, 0);

    for (int i = 0; i < n_moves; ++i) {
        if (moves[i] == m) {
            scores[i] = 9999;
        } else if (moves[i] == board->killers[board->ply][0]) {
            scores[i] = 502;
        } else if (moves[i] == board->killers[board->ply][1]) {
            scores[i] = 501;
        } else if (CAPTURED(moves[i]) != EMPTY) {
            scores[i] = 1000 + MVVLVA(PIECE(moves[i]), CAPTURED(moves[i]));
        } else {
            scores[i] = MIN(
                board->history[board->turn][END(moves[i])][START(moves[i])] -
                    INT_MAX,
                500);
        }
    }
}

static void pickSingleMoves(MOVE *moves, int *scores, int index, int n_moves) {
    int max = scores[index];
    int maxi = index;

    // find max score index
    for (int i = index; i < n_moves; ++i) {
        if (scores[i] > max) {
            max = scores[i];
            maxi = i;
        }
    }

    // move best move to front
    MOVE temp = moves[index];
    moves[index] = moves[maxi];
    moves[maxi] = temp;

    // move score to front
    int scoretemp = scores[index];
    scores[index] = scores[maxi];
    scores[maxi] = scoretemp;
}

// prints uci centipawn/mate search info
static void printEval(int score) {

    if (score + MATETHRESHOLD >= MATE) {
        printf("score mate %d ", (MATE - score + 1) / 2);
    } else if (score - MATETHRESHOLD <= -MATE) {
        printf("score mate %d ", -(MATE + score) / 2);
    } else {
        printf("score cp %d ", score);
    }
}

static void printPV(BOARD_STATE *board, int depth) {
    MOVE list[depth];

    int i = 0;
    while (i < depth) {
        MOVE m;
        int val = probeTT(board->hash, &m, INF, -INF, 0);
        if (val == TT_EMPTY) {
            break;
        }
        printMoveText(m);
        printf(" ");
        makeMove(board, m);
        list[i++] = m;
    }

    while (i > 0) {
        unmakeMove(board, list[--i]);
    }
}

// prints uci search info
static void printInfo(BOARD_STATE *board, float time, int score, int depth) {

    // print search info
    long nps = floor((1000 * board->nodes / time));

    printf("info ");
    printf("depth %d ", depth);
    printEval(score);
    printf("nodes %ld ", board->nodes);
    printf("nps %ld ", nps);
    printf("time %ld ", (long)time);

    printf("pv ");
    printPV(board, depth);
    printf("\n");
    fflush(stdout);
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

static int alphabeta(BOARD_STATE *board, int depth, int alpha, int beta,
                     int doNull) {

    checkTime(board);
    if (board->stopped) {
        return 0;
    }
    ++board->nodes;

    int score = -INF;
    int bestscore = -INF;
    int oldalpha = alpha;

    MOVE bestmove;
    // int flag = TT_ALPHA_FLAG;

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

    int val = probeTT(board->hash, &bestmove, alpha, beta, depth);
    if (val != TT_EMPTY) {
        // hotfix for repetition detection
        makeMove(board, bestmove);
        int rep = isTwoFold(board);
        unmakeMove(board, bestmove);
        if (rep) {
            return 0;
        }

        return val;
    }

    // null move pruning
    if (doNull && depth >= 4 && !incheck) {

        makeNullMove(board);
        int nullScore = -alphabeta(board, depth - 2, -beta, -beta + 1, FALSE);
        unmakeNullMove(board);

        if (nullScore >= beta) {
            return beta;
        }
    }

    MOVE moves[MAX_LEGAL_MOVES];
    int n_moves = generateMoves(board, moves);

    int scores[n_moves];
    scoreMoves(board, moves, scores, n_moves);

    for (int i = 0; i < n_moves; ++i) {

        pickSingleMoves(moves, scores, i, n_moves);

        makeMove(board, moves[i]);
        ++board->ply;

        if (!isAttacked(board, SQ64SQ120(getKingSq(board, !board->turn)),
                        board->turn)) {
            ++legal;

            // full depth search on first move
            if (i == 0) {

                score = -alphabeta(board, depth - 1, -beta, -alpha, doNull);

            }

            // late move reduction
            else {

                if (i >= 4 && board->ply >= 3 && depth >= 2 &&
                    CAPTURED(moves[i]) == EMPTY &&
                    PROMOTED(moves[i]) == EMPTY &&
                    !isAttacked(board, SQ64SQ120(getKingSq(board, board->turn)),
                                !board->turn)) {

                    // shallow search, check if late move could beat alpha
                    score = -alphabeta(board, depth - 2, -alpha - 1, -alpha,
                                       doNull);
                } else {
                    score = alpha + 1;
                }

                // if LMR found better move
                if (score > alpha) {

                    // search again at normal depth, minimal window
                    score = -alphabeta(board, depth - 1, -alpha - 1, -alpha,
                                       doNull);

                    if (score > alpha && score < beta) {
                        score =
                            -alphabeta(board, depth - 1, -beta, -alpha, doNull);
                    }
                }
            }
        }

        unmakeMove(board, moves[i]);
        --board->ply;

        if (score > bestscore) {
            bestscore = score;
            bestmove = moves[i];
            if (score > alpha) {
                if (score >= beta && !board->stopped) {
                    bestmove = moves[i];

                    storeTT(board, moves[i], beta, TT_BETA_FLAG, depth);
                    if (CAPTURED(moves[i]) == EMPTY && doNull) {
                        board->killers[board->ply][1] =
                            board->killers[board->ply][0];
                        board->killers[board->ply][0] = moves[i];

                        board->history[board->turn][END(moves[i])]
                                      [START(moves[i])] += depth * depth;
                    }

                    return beta;
                }
            }
            // flag = TT_EXACT_FLAG;
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

    if (alpha != oldalpha && !board->stopped) {
        storeTT(board, bestmove, bestscore, TT_EXACT_FLAG, depth);
    } else if (!board->stopped) {
        storeTT(board, bestmove, alpha, TT_ALPHA_FLAG, depth);
    }

    return alpha;
}

void printMoveText(MOVE move) {

    char startFile = SQ120F(START120(move)) + 'a';
    char startRank = SQ120R(START120(move)) + '1';
    char endFile = SQ120F(END120(move)) + 'a';
    char endRank = SQ120R(END120(move)) + '1';

    printf("%c%c%c%c", startFile, startRank, endFile, endRank);

    switch (PROMOTED(move)) {
    case bbQueen:
        printf("q");
        break;
    case bbRook:
        printf("r");
        break;
    case bbBishop:
        printf("b");
        break;
    case bbKnight:
        printf("n");
        break;
    }
}

// returns true when next iteration of search should be
// prevented due to time restrictions
int searchCutoff(BOARD_STATE *board, float time_ms) {

    // if used more than half of allocated time,
    // don't start a new search
    return (time_ms * 2 >= board->cutoffTime);
}

int setCutoff(BOARD_STATE *board) {
    int time = inputTime[board->turn];
    int inc = inputInc[board->turn];

    if (inputMovetime != 0) {
        return inputMovetime;
    }

    if (time == DEFAULT_TIME) {
        return 1000 * 60 * 36;
    }

    return (time / 20) + (4 * inc / 5);
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
    if (!board->ponder) {
        board->cutoffTime = setCutoff(board);
    } else {
        board->cutoffTime = 999999999;
    }

    // begin timer
    board->start = clock();

    // if only one legal move, play instantly
    if (!board->ponder && onlyMove(board, &bestmove)) {
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

        // measure time spent
        float new_t = clock() - board->start;
        float time_taken_ms = (1000 * new_t) / CLOCKS_PER_SEC;

        // print search info
        printInfo(board, time_taken_ms, score, searchDepth);

        // get bestmove/ponder from pv
        probeTT(board->hash, &bestmove, INF, -INF, 0);

        // prevent new searches if not enough time
        if (!board->ponder && searchCutoff(board, time_taken_ms)) {
            break;
        }
    }

    // print best move
    printf("bestmove ");
    printMoveText(bestmove);

    makeMove(board, bestmove);

    printf(" ponder ");
    MOVE pmove;
    probeTT(board->hash, &pmove, INF, -INF, 0);
    printMoveText(pmove);
    printf("\n");

    unmakeMove(board, bestmove);

    if (!board->ponder) {
        initTT();
        initHistoryHeuristic(board);
    }
}
