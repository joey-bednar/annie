#include "defs.h"
#include <assert.h>
#include <stdio.h>

// play a move on the board
void makeMove(BOARD_STATE *board, MOVE move) {

    if (move.type == MOVE_QUIET) {
        int piece = getPieceSq120(SQ64SQ120(START(move.compress)), board);

        // move piece to target square
        CLEARBIT(board->bitboard[GENERIC(piece)], START(move.compress));
        CLEARBIT(board->bitboard[board->turn], START(move.compress));
        CLEARBIT(board->bitboard[bbAny], START(move.compress));
        // setPiece120(EMPTY, SQ64SQ120(START(move.compress)), board);
        // setPiece120(piece, SQ64SQ120(END(move.compress)), board);

        SETBIT(board->bitboard[GENERIC(piece)], END(move.compress));
        SETBIT(board->bitboard[board->turn], END(move.compress));
        SETBIT(board->bitboard[bbAny], END(move.compress));

        if (CHECKBIT(board->bitboard[bbKing], END(move.compress))) {
            board->kings[board->turn] = SQ64SQ120(END(move.compress));
            if (board->turn == WHITE) {
                CLEARBIT(board->castle, WK_CASTLE);
                CLEARBIT(board->castle, WQ_CASTLE);
            } else {
                CLEARBIT(board->castle, BK_CASTLE);
                CLEARBIT(board->castle, BQ_CASTLE);
            }
        }

        // update castling permissions if moving a king/rook
        if (piece == wR && SQ64SQ120(START(move.compress)) == H1) {
            CLEARBIT(board->castle, WK_CASTLE);
        } else if (piece == wR && SQ64SQ120(START(move.compress)) == A1) {
            CLEARBIT(board->castle, WQ_CASTLE);
        } else if (piece == bR && SQ64SQ120(START(move.compress)) == H8) {
            CLEARBIT(board->castle, BK_CASTLE);
        } else if (piece == bR && SQ64SQ120(START(move.compress)) == A8) {
            CLEARBIT(board->castle, BQ_CASTLE);
        }
        board->enpassant = OFFBOARD;

        board->turn = !(board->turn);
        return;
    } else if (move.type == MOVE_CAPTURE) {
        int piece = getPieceSq120(SQ64SQ120(START(move.compress)), board);

        // move piece to target square
        setPiece120(EMPTY, SQ64SQ120(START(move.compress)), board);
        setPiece120(piece, SQ64SQ120(END(move.compress)), board);

        // update castling permissions if moving a king/rook
        if (piece == wR && SQ64SQ120(START(move.compress)) == H1) {
            CLEARBIT(board->castle, WK_CASTLE);
        } else if (piece == wR && SQ64SQ120(START(move.compress)) == A1) {
            CLEARBIT(board->castle, WQ_CASTLE);
        } else if (piece == bR && SQ64SQ120(START(move.compress)) == H8) {
            CLEARBIT(board->castle, BK_CASTLE);
        } else if (piece == bR && SQ64SQ120(START(move.compress)) == A8) {
            CLEARBIT(board->castle, BQ_CASTLE);
        } else if (piece == wK && SQ64SQ120(START(move.compress)) == E1) {
            CLEARBIT(board->castle, WK_CASTLE);
            CLEARBIT(board->castle, WQ_CASTLE);
        } else if (piece == bK && SQ64SQ120(START(move.compress)) == E8) {
            CLEARBIT(board->castle, BK_CASTLE);
            CLEARBIT(board->castle, BQ_CASTLE);
        }

        board->enpassant = OFFBOARD;

        board->turn = !(board->turn);
        return;

    } else if (move.type == MOVE_DOUBLEPAWN) {

        int piece = getPieceSq120(SQ64SQ120(START(move.compress)), board);

        // move piece to target square
        setPiece120(EMPTY, SQ64SQ120(START(move.compress)), board);
        setPiece120(piece, SQ64SQ120(END(move.compress)), board);

        // set new en passant square after two square pawn move.
        // delete en passant square if last move wasn't a two square pawn move
        const int offset[2] = {S, N};
        board->enpassant = SQ64SQ120(END(move.compress)) + offset[board->turn];

        board->turn = !(board->turn);
        return;
    }

    // remove kingside/queenside castling ability
    // after castling. place rook in new position.
    else if (move.type == MOVE_KINGCASTLE) {
        if (board->turn == WHITE) {

            setPiece120(EMPTY, SQ64SQ120(START(move.compress)), board);
            setPiece120(wK, SQ64SQ120(END(move.compress)), board);

            setPiece120(wR, F1, board);
            setPiece120(EMPTY, H1, board);
            CLEARBIT(board->castle, WK_CASTLE);
            CLEARBIT(board->castle, WQ_CASTLE);
        } else {

            setPiece120(EMPTY, SQ64SQ120(START(move.compress)), board);
            setPiece120(bK, SQ64SQ120(END(move.compress)), board);

            setPiece120(bR, F8, board);
            setPiece120(EMPTY, H8, board);
            CLEARBIT(board->castle, BK_CASTLE);
            CLEARBIT(board->castle, BQ_CASTLE);
        }

        board->enpassant = OFFBOARD;
        board->turn = !(board->turn);
        return;

    } else if (move.type == MOVE_QUEENCASTLE) {
        if (board->turn == WHITE) {
            setPiece120(EMPTY, SQ64SQ120(START(move.compress)), board);
            setPiece120(wK, SQ64SQ120(END(move.compress)), board);

            setPiece120(wR, D1, board);
            setPiece120(EMPTY, A1, board);
            CLEARBIT(board->castle, WK_CASTLE);
            CLEARBIT(board->castle, WQ_CASTLE);
        } else {

            setPiece120(EMPTY, SQ64SQ120(START(move.compress)), board);
            setPiece120(bK, SQ64SQ120(END(move.compress)), board);

            setPiece120(bR, D8, board);
            setPiece120(EMPTY, A8, board);
            CLEARBIT(board->castle, BK_CASTLE);
            CLEARBIT(board->castle, BQ_CASTLE);
        }

        board->enpassant = OFFBOARD;
        board->turn = !(board->turn);
        return;

    } else if (move.type == MOVE_EPCAPTURE) {
        int piece = getPieceSq120(SQ64SQ120(START(move.compress)), board);

        // move piece to target square
        setPiece120(EMPTY, SQ64SQ120(START(move.compress)), board);
        setPiece120(piece, SQ64SQ120(END(move.compress)), board);
        board->enpassant = OFFBOARD;

        // delete en passant square after performing en passant
        // remove en passanted piece
        const int offset[2] = {S, N};
        board->enpassant = OFFBOARD;
        setPiece120(EMPTY, SQ64SQ120(END(move.compress)) + offset[board->turn],
                    board);
        board->turn = !(board->turn);
        return;
    } else if (move.type >= MOVE_KNIGHTPROMOTE) {
        int piece = getPieceSq120(SQ64SQ120(START(move.compress)), board);

        // move piece to target square
        setPiece120(EMPTY, SQ64SQ120(START(move.compress)), board);
        setPiece120(piece, SQ64SQ120(END(move.compress)), board);

        board->enpassant = OFFBOARD;

        setPiece120(move.promotion, SQ64SQ120(END(move.compress)), board);

        board->turn = !(board->turn);
        return;
    }
}

// undo a move on the board
void unmakeMove(BOARD_STATE *board, MOVE move) {

    if (move.type == MOVE_KINGCASTLE) {
        if (board->turn == BLACK) {
            CLEARBIT(board->bitboard[bbRook], SQ120SQ64(F1));
            CLEARBIT(board->bitboard[bbWhite], SQ120SQ64(F1));
            CLEARBIT(board->bitboard[bbAny], SQ120SQ64(F1));
            SETBIT(board->bitboard[bbRook], SQ120SQ64(H1));
            SETBIT(board->bitboard[bbWhite], SQ120SQ64(H1));
            SETBIT(board->bitboard[bbAny], SQ120SQ64(H1));
        } else {

            CLEARBIT(board->bitboard[bbRook], SQ120SQ64(F8));
            CLEARBIT(board->bitboard[bbBlack], SQ120SQ64(F8));
            CLEARBIT(board->bitboard[bbAny], SQ120SQ64(F8));
            SETBIT(board->bitboard[bbRook], SQ120SQ64(H8));
            SETBIT(board->bitboard[bbBlack], SQ120SQ64(H8));
            SETBIT(board->bitboard[bbAny], SQ120SQ64(H8));
        }
    } else if (move.type == MOVE_QUEENCASTLE) {
        if (board->turn == BLACK) {
            CLEARBIT(board->bitboard[bbRook], SQ120SQ64(D1));
            CLEARBIT(board->bitboard[bbWhite], SQ120SQ64(D1));
            CLEARBIT(board->bitboard[bbAny], SQ120SQ64(D1));
            SETBIT(board->bitboard[bbRook], SQ120SQ64(A1));
            SETBIT(board->bitboard[bbWhite], SQ120SQ64(A1));
            SETBIT(board->bitboard[bbAny], SQ120SQ64(A1));
        } else {

            CLEARBIT(board->bitboard[bbRook], SQ120SQ64(D8));
            CLEARBIT(board->bitboard[bbBlack], SQ120SQ64(D8));
            CLEARBIT(board->bitboard[bbAny], SQ120SQ64(D8));
            SETBIT(board->bitboard[bbRook], SQ120SQ64(A8));
            SETBIT(board->bitboard[bbBlack], SQ120SQ64(A8));
            SETBIT(board->bitboard[bbAny], SQ120SQ64(A8));
        }
    }

    // reset castling abilities
    board->castle = move.priorcastle;

    int offset = S;
    if (board->turn == WHITE) {
        offset = N;
    }

    int piece = getPieceSq120(SQ64SQ120(END(move.compress)), board);

    // perform en passant
    if (move.type == MOVE_EPCAPTURE) {
        board->enpassant = SQ64SQ120(END(move.compress));
        setPiece120(move.captured, SQ64SQ120(END(move.compress)) + offset,
                    board);
        setPiece120(EMPTY, SQ64SQ120(END(move.compress)), board);
        setPiece120(piece, SQ64SQ120(START(move.compress)), board);
        board->turn = !(board->turn);
        return;
    }

    // put captured piece back
    setPiece120(move.captured, SQ64SQ120(END(move.compress)), board);

    // undo promotion
    if (move.promotion == EMPTY) {
        setPiece120(piece, SQ64SQ120(START(move.compress)), board);
    } else {
        int pawn = wP;
        if (COLOR(piece) == BLACK) {
            pawn = bP;
        }
        setPiece120(pawn, SQ64SQ120(START(move.compress)), board);
    }

    board->enpassant = move.priorep;

    board->turn = !(board->turn);
}

// add a move to the provided moves array
static void addMove(BOARD_STATE *board, MOVE *moves, int type, int start,
                    int end, int captured, int promotion, int *index) {
    // moves[*index].piece = piece;
    moves[*index].type = type;
    // moves[*index].startSquare = start;
    // moves[*index].endSquare = end;
    moves[*index].captured = captured;
    moves[*index].promotion = promotion;
    moves[*index].priorep = board->enpassant;
    moves[*index].priorcastle = board->castle;

    // 0000 0000 0000 0000 0000 0000 0111 1111 : start
    // 0000 0000 0000 0000 0001 1111 1000 0000 : end
    // 0000 0000 0000 0001 1110 0000 0000 0000 : captured piece
    // 0000 0000 0001 1110 1000 0000 0000 0000 : promoted piece
    // 0000 0000 0010 0000 1000 0000 0000 0000 : en passant

    int ep;
    if (type == MOVE_EPCAPTURE) {
        ep = TRUE;
    } else {
        ep = FALSE;
    }

    moves[*index].compress =
        (((unsigned long)SQ120SQ64(start) << 0) & 0x0000007Ful) |
        (((unsigned long)SQ120SQ64(end) << 7) & 0x00001F80ul) |
        (((unsigned long)GENERIC(captured) << 14) & 0x0001E000ul) |
        (((unsigned long)GENERIC(promotion) << 18) & 0x001E0000ul) |
        (((unsigned long)ep << 19) & 0x00200000ul);

    (*index)++;

    // compression
    // prior castle: 4
    // priorep: 6
    // priorep?: 1
    // captured: 3
    // end: 6
    // start: 6
    // type: 4

    // types
    // simple
    // capture simple
    // en passant capture
    // two pawn move
    // simple promotion
    // capture promotion
    // castle 4 types
}

// same as generateSimpleMoves() but continues in offset direction until not
// possible. Used for sliding pieces (bishop,rook,queen)
static void generateSlidingMoves(BOARD_STATE *board, MOVE *moves, int sq,
                                 int *index, int *offsets, int offsetssize) {

    for (int i = 0; i < offsetssize; i++) {
        int nextSq = sq + offsets[i];

        // if square is empty or can capture enemy piece, it is a pseudolegal
        // move

        while (isEmptySquare(nextSq, board)) {
            addMove(board, moves, MOVE_QUIET, sq, nextSq, EMPTY, EMPTY, index);

            nextSq = nextSq + offsets[i];
        }

        if (ONBOARD(nextSq) &&
            CHECKBIT(board->bitboard[(!board->turn)], SQ120SQ64(nextSq))) {
            int squareContains = getPieceSq120(nextSq, board);
            addMove(board, moves, MOVE_CAPTURE, sq, nextSq, squareContains,
                    EMPTY, index);
        }
    }
}

// add all legal castling moves to the moves array
static void generateCastleMoves(BOARD_STATE *board, MOVE *moves, int *index) {

    int rank = RANK_1;
    int queenside = WQ_CASTLE;
    int kingside = WK_CASTLE;
    if (board->turn == BLACK) {
        rank = RANK_8;
        queenside = BQ_CASTLE;
        kingside = BK_CASTLE;
    }

    int color = board->turn;

    if (isAttacked(board, board->kings[color], !color)) {
        return;
    }

    ULL bb = board->bitboard[bbRook] & board->bitboard[color];

    if (board->kings[color] == FR2SQ120(FILE_E, rank)) {
        if (CHECKBIT(board->castle, kingside) &&
            CHECKBIT(bb, FR2SQ64(FILE_H, rank))) {

            int throughcheck =
                isAttacked(board, FR2SQ120(FILE_F, rank), !color) ||
                isAttacked(board, FR2SQ120(FILE_G, rank), !color);

            int blocked =
                CHECKBIT(board->bitboard[bbAny], FR2SQ64(FILE_F, rank)) ||
                CHECKBIT(board->bitboard[bbAny], FR2SQ64(FILE_G, rank));

            if (!throughcheck && !blocked) {
                addMove(board, moves, MOVE_KINGCASTLE, FR2SQ120(FILE_E, rank),
                        FR2SQ120(FILE_G, rank), EMPTY, EMPTY, index);
            }
        }

        if (CHECKBIT(board->castle, queenside) &&
            CHECKBIT(bb, FR2SQ64(FILE_A, rank))) {

            int throughcheck =
                isAttacked(board, FR2SQ120(FILE_C, rank), !color) ||
                isAttacked(board, FR2SQ120(FILE_D, rank), !color);

            int blocked =
                CHECKBIT(board->bitboard[bbAny], FR2SQ64(FILE_B, rank)) ||
                CHECKBIT(board->bitboard[bbAny], FR2SQ64(FILE_C, rank)) ||
                CHECKBIT(board->bitboard[bbAny], FR2SQ64(FILE_D, rank));

            if (!throughcheck && !blocked) {
                addMove(board, moves, MOVE_QUEENCASTLE, FR2SQ120(FILE_E, rank),
                        FR2SQ120(FILE_C, rank), EMPTY, EMPTY, index);
            }
        }
    }
}

// used by generatePseudoPawnMoves. add all four possible promotion moves to
// moves list.
static void addPromotions(BOARD_STATE *board, MOVE *moves, int start, int end,
                          int captured, int color, int *index) {
    int promoteTo[4] = {wQ, wN, wR, wB};
    int type[4] = {MOVE_QUEENPROMOTE, MOVE_KNIGHTPROMOTE, MOVE_ROOKPROMOTE,
                   MOVE_BISHOPPROMOTE};

    if (captured != EMPTY) {
        type[0] = MOVE_QUEENPROMOTECAPTURE;
        type[1] = MOVE_KNIGHTPROMOTECAPTURE;
        type[2] = MOVE_ROOKPROMOTECAPTURE;
        type[3] = MOVE_BISHOPPROMOTECAPTURE;
    }

    if (color == BLACK) {
        promoteTo[0] = bQ;
        promoteTo[1] = bN;
        promoteTo[2] = bR;
        promoteTo[3] = bB;
    }
    for (int i = 0; i < 4; i++) {
        addMove(board, moves, type[i], start, end, captured, promoteTo[i],
                index);
    }
}

// add all pseudolegal pawn moves to moves list
static void generatePseudoPawnMoves(BOARD_STATE *board, MOVE *moves, int sq,
                                    int *index) {
    int color = board->turn;

    int secondrank = RANK_2;
    int eighthrank = RANK_8;
    int offset[4] = {1, 2, -9, 11};
    int enemypawn = bP;
    if (color == BLACK) {
        offset[0] = -1;
        offset[1] = -2;
        offset[2] = 9;
        offset[3] = -11;
        secondrank = RANK_7;
        eighthrank = RANK_1;
        enemypawn = wP;
    }

    int one = sq + offset[0];
    int two = sq + offset[1];

    // up one
    if (isEmptySquare(one, board)) {
        if (SQ120R(one) == eighthrank) {
            addPromotions(board, moves, sq, one, EMPTY, color, index);
        } else {
            addMove(board, moves, MOVE_QUIET, sq, one, EMPTY, EMPTY, index);
        }
    }

    // captures and en passant
    for (int i = 2; i <= 3; i++) {
        int enemysquare = sq + offset[i];

        if (!ONBOARD(enemysquare)) {
            continue;
        }

        if (CHECKBIT(board->bitboard[(!board->turn)], SQ120SQ64(enemysquare))) {
            int enemypiece = getPieceSq120(enemysquare, board);

            if (SQ120R(enemysquare) == eighthrank) {
                addPromotions(board, moves, sq, enemysquare, enemypiece, color,
                              index);
            } else {
                addMove(board, moves, MOVE_CAPTURE, sq, enemysquare, enemypiece,
                        EMPTY, index);
            }
        } else if (epMap[enemysquare] && enemysquare == board->enpassant) {

            addMove(board, moves, MOVE_EPCAPTURE, sq, enemysquare, enemypawn,
                    EMPTY, index);
        }
    }

    // up two
    if (SQ120R(sq) == secondrank && isEmptySquare(one, board) &&
        isEmptySquare(two, board)) {
        addMove(board, moves, MOVE_DOUBLEPAWN, sq, two, EMPTY, EMPTY, index);
    }
}

// use pregenerated attack bitboards to generate all pseudolegal moves
// and add to array.
static void generatePseudoPresetMoves(BOARD_STATE *board, MOVE *moves, int sq,
                                      ULL bitboard, int *index) {
    ULL bb = ~board->bitboard[board->turn] & bitboard;

    while (bb != 0) {
        int nextSq64 = bitScanForward(bb);
        int nextSq120 = SQ64SQ120(nextSq64);

        int squareContains = getPieceSq120(nextSq120, board);
        int type = MOVE_CAPTURE;
        if (squareContains == EMPTY) {
            type = MOVE_QUIET;
        }
        addMove(board, moves, type, sq, nextSq120, squareContains, EMPTY,
                index);

        ULL mask = (~1ULL << (nextSq64));
        bb &= mask;
    }
}

static void generatePseudoRookMoves(BOARD_STATE *board, MOVE *moves, int sq,
                                    int *index) {
    int offsets[4] = {-10, -1, 10, 1};
    generateSlidingMoves(board, moves, sq, index, offsets, 4);
}

static void generatePseudoBishopMoves(BOARD_STATE *board, MOVE *moves, int sq,
                                      int *index) {
    int offsets[4] = {-11, -9, 9, 11};
    generateSlidingMoves(board, moves, sq, index, offsets, 4);
}

// generate all legal moves and insert them into the moves list
int generateMoves(BOARD_STATE *board, MOVE *moves) {

    int index = 0;

    // only calc white/black moves on white/black's turn
    ULL bb = board->bitboard[board->turn];

    while (bb != 0) {
        int index64 = bitScanForward(bb);
        int sq = SQ64SQ120(index64);

        int piece = getGenericPieceSq120(sq, board);

        ULL mask = (~1ULL << (index64));
        bb &= mask;

        switch (piece) {
        case bbPawn:
            generatePseudoPawnMoves(board, moves, sq, &index);
            break;
        case bbRook:
            generatePseudoRookMoves(board, moves, sq, &index);
            break;
        case bbBishop:
            generatePseudoBishopMoves(board, moves, sq, &index);
            break;
        case bbKnight:
            generatePseudoPresetMoves(board, moves, sq, KNIGHTBB(SQ120SQ64(sq)),
                                      &index);
            break;
        case bbQueen:
            generatePseudoRookMoves(board, moves, sq, &index);
            generatePseudoBishopMoves(board, moves, sq, &index);
            break;
        case bbKing:
            generatePseudoPresetMoves(board, moves, sq, KINGBB(SQ120SQ64(sq)),
                                      &index);
            break;
        }
    }

    generateCastleMoves(board, moves, &index);

    return index;
}
