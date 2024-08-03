#include "defs.h"
#include <assert.h>
#include <stdlib.h>

// generates random ULL
static ULL get64rand() {
    return (((ULL)rand() << 0) & 0x000000000000FFFFull) |
           (((ULL)rand() << 16) & 0x00000000FFFF0000ull) |
           (((ULL)rand() << 32) & 0x0000FFFF00000000ull) |
           (((ULL)rand() << 48) & 0xFFFF000000000000ull);
}

// init random zobrist values
void initZobrist() {

    srand(2);

    for (int piece = wP; piece <= bK; ++piece) {
        for (int sq = 0; sq < 64; ++sq) {
            zobrist_vals[piece - 1][sq] = get64rand();
        }
    }

    zobristB2M = get64rand();

    for (int i = 0; i < 32; i++) {
        zobristCastle[i] = get64rand();
    }

    for (int i = 0; i < 64; i++) {
        zobristEnpassant[i] = get64rand();
    }
}

// constructs hash of position
void loadZobrist(BOARD_STATE *board) {
    board->hash = 0ULL;
    if (board->turn == BLACK) {
        board->hash ^= zobristB2M;
    }
    ULL bb = board->bitboard[WHITE] | board->bitboard[BLACK];
    BITLOOP(bb) {
        int sq64 = bitScanForward(bb);
        int piece = getPieceSq120(SQ64SQ120(sq64), board);
        board->hash ^= zobrist_vals[piece - 1][sq64];
    }

    for (int i = 0; i < 2 * MAX_GAME_LENGTH; ++i) {
        board->playedmoves[i].hash = 0ull;
    }
}

// updates zobrist hash after placing/removing a piece from square
void updateZobrist(int sq64, int piece, BOARD_STATE *board) {
    board->hash ^= zobrist_vals[piece - 1][sq64];
}

// updates zobrist hash with new turn
void turnZobrist(BOARD_STATE *board) {
    board->hash ^= zobristB2M;
}

// updates zobrist hash with castle permissions
void castleZobrist(BOARD_STATE *board) {
    board->hash ^= zobristCastle[board->castle];
}

// updates zobrist hash with en passant square
void epZobrist(BOARD_STATE *board) {
    if (ONBOARD(board->enpassant)) {
        board->hash ^= zobristEnpassant[board->enpassant];
    } else {
        board->hash ^= zobristEnpassant[0];
    }
}
