#include "eval.h"
#include "board.h"
#include "hash.h"
#include "misc.h"
#include "utils.h"

#define PASSED_PAWN_BONUS 30
#define BISHOP_PAIR_BONUS 30

#define PAWN_MATERIAL 100
#define KNIGHT_MATERIAL 320
#define BISHOP_MATERIAL 330
#define ROOK_MATERIAL 500
#define QUEEN_MATERIAL 900

#define PAWN_PHASE 0
#define KNIGHT_PHASE 1
#define BISHOP_PHASE 1
#define ROOK_PHASE 2
#define QUEEN_PHASE 4

// returns true if position has been repeated three times
int isTwoFold(BOARD_STATE *board) {
    if (board->halfmove >= 2) {
        int end = board->pmindex;
        int start = 0; // end - board->halfmove;
        int count = 0;
        for (int i = start; i <= end; ++i) {
            if ((board->hash ^ zobristB2M) == board->playedmoves[i].hash) {
                ++count;
            }
        }

        if (count >= 2) {
            return TRUE;
        }
    }

    return FALSE;
}

// returns true if position has been repeated three times
int isThreeFold(BOARD_STATE *board) {
    if (board->halfmove >= 4) {
        int end = board->pmindex;
        int start = 0; // end - board->halfmove;
        int count = 0;
        for (int i = start; i <= end; ++i) {
            if ((board->hash ^ zobristB2M) == board->playedmoves[i].hash) {
                ++count;
            }
        }

        if (count >= 3) {
            return TRUE;
        }
    }

    return FALSE;
}

int isInsufficientMaterial(BOARD_STATE *board) {

    // any queen/rook/pawn can mate
    if (board->bitboard[bbQueen] > 0ull || board->bitboard[bbRook] > 0ull ||
        board->bitboard[bbPawn] > 0ull) {
        return FALSE;
    }

    ULL whiteBishop = board->bitboard[bbBishop] & board->bitboard[WHITE];
    ULL blackBishop = board->bitboard[bbBishop] & board->bitboard[BLACK];

    ULL whiteKnight = board->bitboard[bbKnight] & board->bitboard[WHITE];
    ULL blackKnight = board->bitboard[bbKnight] & board->bitboard[BLACK];

    ULL white = (board->bitboard[bbBishop] | board->bitboard[bbKnight]) &
                board->bitboard[WHITE];
    ULL black = (board->bitboard[bbBishop] | board->bitboard[bbKnight]) &
                board->bitboard[BLACK];

    // K minor vs K minor
    if (white > 0ull && black > 0ull) {
        return FALSE;
    }

    // bishop/knight can mate
    if (whiteBishop > 0ull && whiteKnight > 0ull) {
        return FALSE;
    }
    if (blackBishop > 0ull && blackKnight > 0ull) {
        return FALSE;
    }

    // bishop pair can mate
    int whiteBishopCount = countBits(whiteBishop);
    int blackBishopCount = countBits(blackBishop);
    if (whiteBishopCount > 1 || blackBishopCount > 1) {
        return FALSE;
    }

    // FIDE rules KNN vs K not forced draw
    int whiteKnightCount = countBits(whiteKnight);
    int blackKnightCount = countBits(blackKnight);
    if (whiteKnightCount > 1 || blackKnightCount > 1) {
        return FALSE;
    }

    return TRUE;
}

static int bonusPassedPawn(BOARD_STATE *board, int sq64, int color) {

    ULL bb = (PASSEDPAWN(sq64, color) & board->bitboard[bbPawn] &
              board->bitboard[!color]);

    if (bb != 0ull) {
        return 0;
    }

    return PASSED_PAWN_BONUS;
}

static int bonusBishopPair(BOARD_STATE *board, int color) {
    if (countBits(board->bitboard[bbBishop] & board->bitboard[color]) >= 2) {
        return BISHOP_PAIR_BONUS;
    }
    return 0;
}

static int computeMopUp(int winningcolor, BOARD_STATE *board) {
    // clang-format off
    const int centerManhattanDistance[64] = {
      6, 5, 4, 3, 3, 4, 5, 6,
      5, 4, 3, 2, 2, 3, 4, 5,
      4, 3, 2, 1, 1, 2, 3, 4,
      3, 2, 1, 0, 0, 1, 2, 3,
      3, 2, 1, 0, 0, 1, 2, 3,
      4, 3, 2, 1, 1, 2, 3, 4,
      5, 4, 3, 2, 2, 3, 4, 5,
      6, 5, 4, 3, 3, 4, 5, 6
    };
    // clang-format on

    int CMD = centerManhattanDistance[getKingSq(board, !winningcolor)];
    int MD =
        manhattanDistance(getKingSq(board, WHITE), getKingSq(board, BLACK));

    int mopUp = (47 * CMD + 16 * (14 - MD));

    return mopUp;
}

// returns phase of game as integer 0-256
// 0 = opening, 256 = endgame
int getPhase(BOARD_STATE *board) {
    ULL white = board->bitboard[bbWhite];
    ULL black = board->bitboard[bbBlack];

    const int TOTAL_PHASE = 16 * PAWN_PHASE + 4 * KNIGHT_PHASE +
                            4 * BISHOP_PHASE + 4 * ROOK_PHASE + 2 * QUEEN_PHASE;

    int phase = TOTAL_PHASE;

    phase -= PAWN_PHASE * countBits(white & board->bitboard[bbPawn]) +
             PAWN_PHASE * countBits(black & board->bitboard[bbPawn]) +

             KNIGHT_PHASE * countBits(white & board->bitboard[bbKnight]) +
             KNIGHT_PHASE * countBits(black & board->bitboard[bbKnight]) +

             BISHOP_PHASE * countBits(white & board->bitboard[bbBishop]) +
             BISHOP_PHASE * countBits(black & board->bitboard[bbBishop]) +

             ROOK_PHASE * countBits(white & board->bitboard[bbRook]) +
             ROOK_PHASE * countBits(black & board->bitboard[bbRook]) +

             QUEEN_PHASE * countBits(white & board->bitboard[bbQueen]) +
             QUEEN_PHASE * countBits(black & board->bitboard[bbQueen]);

    return (phase * 256 + (TOTAL_PHASE / 2)) / TOTAL_PHASE;
}

static void computeSquares(BOARD_STATE *board, int color, int *pieces,
                           int *open, int *end) {

    const int val[] = {PAWN_MATERIAL, KNIGHT_MATERIAL, BISHOP_MATERIAL,
                       ROOK_MATERIAL, QUEEN_MATERIAL,  0};
    int total = 0;

    ULL bb = board->bitboard[color];

    BITLOOP(bb) {
        int index64 = bitScanForward(bb);
        int unflipped64 = index64;

        int sq = SQ64SQ120(index64);

        int piece = getGenericPieceSq120(sq, board);

        if (color == BLACK) {
            index64 ^= 7;
        }

        *pieces += val[piece - bbPawn];

        switch (piece) {
        case bbPawn:
            *open += pawnSqTable[FALSE][index64];
            *end += pawnSqTable[TRUE][index64];
            total += bonusPassedPawn(board, unflipped64, color);
            break;
        case bbRook:
            *open += rookSqTable[FALSE][index64];
            *end += rookSqTable[TRUE][index64];
            break;
        case bbBishop:
            *open += bishopSqTable[FALSE][index64];
            *end += bishopSqTable[TRUE][index64];
            break;
        case bbKnight:
            *open += knightSqTable[FALSE][index64];
            *end += knightSqTable[TRUE][index64];
            break;
        case bbQueen:
            *open += queenSqTable[FALSE][index64];
            *end += queenSqTable[TRUE][index64];
            break;
        case bbKing:
            *open += kingSqTable[FALSE][index64];
            *end += kingSqTable[TRUE][index64];
            break;
        }
    }

    *open += total;
    *end += total;
}

// returns centipawn evaluation of current position
int eval(BOARD_STATE *board) {

    int wmaterial = 0;
    int wopen = 0;
    int wend = 0;
    computeSquares(board, board->turn, &wmaterial, &wopen, &wend);

    int bmaterial = 0;
    int bopen = 0;
    int bend = 0;
    computeSquares(board, !board->turn, &bmaterial, &bopen, &bend);

    int totalMiddle = wopen - bopen;
    int totalEnd = wend - bend;

    int phase = getPhase(board);

    int eval = ((totalMiddle * (256 - phase)) + (totalEnd * phase)) / 256;

    eval += wmaterial - bmaterial;

    eval += bonusBishopPair(board, board->turn) -
            bonusBishopPair(board, !board->turn);

    // compute mop up eval for endgames
    int endgame =
        (wmaterial + bmaterial < 2500) || board->bitboard[bbQueen] == 0ull;
    if (endgame) {
        if (eval > 400) {
            eval += computeMopUp(board->turn, board);
        } else if (eval < -400) {
            eval -= computeMopUp(!board->turn, board);
        }
    }

    return eval;
}
