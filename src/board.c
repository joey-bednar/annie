#include "board.h"
#include "hash.h"
#include "misc.h"
#include "moves.h"
#include "utils.h"
#include <assert.h>

// clears the chess board of any pieces
void clearBoard(BOARD_STATE *board) {

    for (int i = 0; i < bbLength; ++i) {
        board->bitboard[i] = 0ull;
    }

    board->turn = WHITE;
    board->enpassant = OFFBOARD;
    board->halfmove = 0;
    board->fullmove = 1;
    board->pmindex = 0;
    CLEARBIT(board->castle, WK_CASTLE);
    CLEARBIT(board->castle, WQ_CASTLE);
    CLEARBIT(board->castle, BK_CASTLE);
    CLEARBIT(board->castle, BQ_CASTLE);
}

// sets up the pieces for a new game
void initBoard(BOARD_STATE *board) {

    clearBoard(board);

    // add white pieces
    setPiece(wR, FILE_A, RANK_1, board);
    setPiece(wN, FILE_B, RANK_1, board);
    setPiece(wB, FILE_C, RANK_1, board);
    setPiece(wQ, FILE_D, RANK_1, board);
    setPiece(wK, FILE_E, RANK_1, board);
    setPiece(wB, FILE_F, RANK_1, board);
    setPiece(wN, FILE_G, RANK_1, board);
    setPiece(wR, FILE_H, RANK_1, board);

    // add black pieces
    setPiece(bR, FILE_A, RANK_8, board);
    setPiece(bN, FILE_B, RANK_8, board);
    setPiece(bB, FILE_C, RANK_8, board);
    setPiece(bQ, FILE_D, RANK_8, board);
    setPiece(bK, FILE_E, RANK_8, board);
    setPiece(bB, FILE_F, RANK_8, board);
    setPiece(bN, FILE_G, RANK_8, board);
    setPiece(bR, FILE_H, RANK_8, board);

    // add pawns
    for (int file = FILE_A; file <= FILE_H; ++file) {
        setPiece(wP, file, RANK_2, board);
        setPiece(bP, file, RANK_7, board);
    }

    board->turn = WHITE;
    board->enpassant = OFFBOARD;
    board->halfmove = 0;
    board->fullmove = 1;
    board->pmindex = 0;

    SETBIT(board->castle, WK_CASTLE);
    SETBIT(board->castle, WQ_CASTLE);
    SETBIT(board->castle, BK_CASTLE);
    SETBIT(board->castle, BQ_CASTLE);

    loadZobrist(board);
}

void clearPiece(BOARD_STATE *board, int piece, int sq) {
    // move piece to target square
    CLEARBIT(board->bitboard[GENERIC(piece)], SQ120SQ64(sq));
    CLEARBIT(board->bitboard[COLOR(piece)], SQ120SQ64(sq));
    updateZobrist(SQ120SQ64(sq), piece, board);
}

void placePiece(BOARD_STATE *board, int piece, int sq) {

    SETBIT(board->bitboard[GENERIC(piece)], SQ120SQ64(sq));
    SETBIT(board->bitboard[COLOR(piece)], SQ120SQ64(sq));
    updateZobrist(SQ120SQ64(sq), piece, board);
}

int getKingSq(BOARD_STATE *board, int color) {
    ULL kings = board->bitboard[color] & board->bitboard[bbKing];
    int sq = bitScanForward(kings);
    return sq;
}

void updateCastling(BOARD_STATE *board, MOVE move) {

    int piece = getGenericPieceSq120(START120(move), board);

    if (piece == bbRook) {
        if (START120(move) == H1) {
            CLEARBIT(board->castle, WK_CASTLE);
        } else if (START120(move) == A1) {
            CLEARBIT(board->castle, WQ_CASTLE);
        } else if (START120(move) == H8) {
            CLEARBIT(board->castle, BK_CASTLE);
        } else if (START120(move) == A8) {
            CLEARBIT(board->castle, BQ_CASTLE);
        }
    } else if (piece == bbKing) {
        if (START120(move) == E1) {
            CLEARBIT(board->castle, WK_CASTLE);
            CLEARBIT(board->castle, WQ_CASTLE);
        } else if (END120(move) == E8) {
            CLEARBIT(board->castle, BK_CASTLE);
            CLEARBIT(board->castle, BQ_CASTLE);
        }
    }
}

int isEmptySquare(int sq, BOARD_STATE *board) {
    ULL bb = (board->bitboard[bbWhite] | board->bitboard[bbBlack]);
    ULL val = CHECKBIT(bb, SQ120SQ64(sq));

    return (ONBOARD(sq) && !val);
}

int getGenericPieceSq120(int sq, BOARD_STATE *board) {
    int sq64 = SQ120SQ64(sq);
    for (int i = bbPawn; i <= bbKing; ++i) {
        if (CHECKBIT(board->bitboard[i], sq64)) {
            return i;
        };
    }
    return EMPTY;
}

// return piece on square given by 120 index
int getPieceSq120(int sq, BOARD_STATE *board) {

    if (!ONBOARD(sq)) {
        return OFFBOARD;
    }

    int sq64 = SQ120SQ64(sq);

    ULL set =
        CHECKBIT(board->bitboard[bbWhite] | board->bitboard[bbBlack], sq64);

    if (!set) {
        return EMPTY;
    }

    for (int i = bbPawn; i <= bbKing; ++i) {

        ULL val = CHECKBIT(board->bitboard[i], sq64);
        if (val) {
            if (CHECKBIT((board->bitboard[bbWhite]), sq64)) {
                return TOWHITE(i);
            }
            return TOBLACK(i);
        }
    }

    // shouldn't get here
    return OFFBOARD;
}

void setPiece120(int piece, int sq, BOARD_STATE *board) {

    const int color = COLOR(piece);
    const int generic = GENERIC(piece);

    int sq64 = SQ120SQ64(sq);

    for (int i = 0; i < bbLength; ++i) {
        CLEARBIT(board->bitboard[i], sq64);
    }

    // add piece to bitboard
    if (piece != EMPTY) {
        SETBIT(board->bitboard[generic], sq64);
        SETBIT(board->bitboard[color], sq64);
    }
}

// place piece on given square by updating board/bitboards
void setPiece(int piece, int file, int rank, BOARD_STATE *board) {

    int sq = FR2SQ120(file, rank);
    setPiece120(piece, sq, board);
}
