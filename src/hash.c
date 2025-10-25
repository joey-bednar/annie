#include <assert.h>
#include <stdlib.h>

#include "board.h"
#include "hash.h"
#include "misc.h"
#include "utils.h"

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

    // compute hash for all piece/square combinations
    for (int piece = wP; piece <= bK; ++piece) {
        for (int sq = 0; sq < 64; ++sq) {
            zobrist_vals[piece - 1][sq] = get64rand();
        }
    }

    // compute turn hash
    zobristB2M = get64rand();

    // compute en passant hashes
    ULL offboardHash = 0ull;
    for (int sq120 = 0; sq120 < 120; ++sq120) {
        if (ONBOARD(sq120)) {
            zobristEP[sq120] = get64rand();
        } else {
            zobristEP[sq120] = offboardHash;
        }
    }

    zobristC[0] = 0ull;
    for (int i = 1; i < 64; ++i) {
        zobristC[i] = get64rand();
    }
}

// constructs hash of position
void loadZobrist(BOARD_STATE *board) {
    board->hash = 0ull;
    if (board->turn == BLACK) {
        board->hash ^= zobristB2M;
    }
    ULL bb = board->bitboard[WHITE] | board->bitboard[BLACK];
    BITLOOP(bb) {
        int sq64 = bitScanForward(bb);
        int piece = getPieceSq120(SQ64SQ120(sq64), board);
        board->hash ^= zobrist_vals[piece - 1][sq64];
    }

    board->hash =
        board->hash ^ zobristEP[board->enpassant] ^ zobristC[board->castle];

    for (int i = 0; i < 2 * MAX_GAME_LENGTH; ++i) {
        board->playedmoves[i].hash = 0ull;
    }
}

void updateZobristEp(int start, int end, BOARD_STATE *board) {
    board->hash = board->hash ^ zobristEP[start] ^ zobristEP[end];
    board->enpassant = end;
}

void updateZobristCastle(int start, int end, BOARD_STATE *board) {
    board->hash = board->hash ^ zobristC[start] ^ zobristC[end];
}

// updates zobrist hash after placing/removing a piece from square
void updateZobrist(int sq64, int piece, BOARD_STATE *board) {
    board->hash ^= zobrist_vals[piece - 1][sq64];
}

// updates zobrist hash with new turn
void turnZobrist(BOARD_STATE *board) {
    board->hash ^= zobristB2M;
}
