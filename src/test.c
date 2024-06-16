#include "defs.h"
#include <assert.h>
#include <stdio.h>

static void printAllBoards() {
    BOARD_STATE board;

    // empty board
    clearBoard(&board);
    printBoard(&board);
    printf("\n");

    // starting board
    initBoard(&board);
    printBoard(&board);
    printf("\n");

    // board 120 index
    printBoardIndex();
    printf("\n");

    // sq to file
    for (int rank = RANK_8; rank >= RANK_1; rank--) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            int sq = FR2SQ120(file, rank);
            printf("%d%d ", SQ120F(sq), file);
        }
        printf("\n");
    }
    printf("\n");

    // sq to rank
    for (int rank = RANK_8; rank >= RANK_1; rank--) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            int sq = FR2SQ120(file, rank);
            printf("%d%d ", SQ120R(sq), rank);
        }
        printf("\n");
    }
}

static void testScan() {
    BOARD_STATE board;
    // init legal moves array
    MOVE moves[MAX_LEGAL_MOVES];

    for (int rank = RANK_8; rank >= RANK_1; rank--) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            int sq = FR2SQ120(file, rank);
            clearBoard(&board);
            setPiece120(wK, sq, &board);
            ULL all = (board.bitboard[bbWhite] | board.bitboard[bbBlack]);
            printBitboard(all);
            ULL bb = all;
            int result = SQ64SQ120(bitScanForward(bb));
            printf("result: %d\n", result);
            assert(result == sq);
        }
        printf("\n");
    }
}

static void testScanAll() {
    BOARD_STATE board;
    // init legal moves array
    MOVE moves[MAX_LEGAL_MOVES];

    initBoard(&board);

    ULL bb = (board.bitboard[bbWhite] | board.bitboard[bbBlack]);

    while (bb != 0) {
        printf("board:\n");
        printBitboard(bb);
        printBits(bb);
        printf("\n");

        int index64 = bitScanForward(bb);
        int result = SQ64SQ120(index64);
        printf("index: %d\n", result);

        printf("mask:\n");
        ULL mask = (~1ULL << (index64));
        printBitboard(mask);
        printBits(mask);
        printf("\n");
        bb &= mask;
    }
}

void test() {
    testScanAll();
}
