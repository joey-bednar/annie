#ifndef DEFS_H

#define FALSE 0
#define TRUE 1

#define SQ120R(sq) (((sq) % 10) - 1)
#define SQ120F(sq) (((sq) - 21) / (10))
#define FR2SQ120(f, r) ((21 + (r)) + ((f) * 10))
#define FR2SQ64(f, r) ((r) + ((f) * 8))
#define SQ120SQ64(sq) (sq120sq64Map[(sq)])
#define SQ64SQ120(sq) (sq64sq120Map[(sq)])

#define COLOR(p) (colorMap[(p)])
#define NOTCOLOR(p) (notcolorMap[(p)])

#define GENERIC(p) (genericMap[(p)])
#define TOWHITE(p) (toWhite[(p)])
#define TOBLACK(p) (toBlack[(p)])
#define ONBOARD(sq) (onboardMap[(sq)])

#define CHECKBIT(bb, sq64) (((bb)) >> ((sq64)) & 1ULL)
#define CLEARBIT(bb, sq64) ((bb) &= ~(1ULL << sq64))
#define SETBIT(bb, sq64) ((bb) |= (1ULL << sq64))
#define CLEARBITBOARD(bb) ((bb) = 0)
#define KNIGHTBB(sq64) (knightJumps[(sq64)])
#define KINGBB(sq64) (kingJumps[(sq64)])

#define MAX_LEGAL_MOVES 256
#define MAX_DEPTH 10

#define CHAR2FILE(c) ((int)((c) - 'a'))
#define CHAR2RANK(c) ((int)((c) - '1'))
#define CHAR2SQ120(f, r) (FR2SQ120((CHAR2FILE((f))), (CHAR2RANK((r)))))

#define START(move) (int)((move) & 0x7F)
#define END(move) (int)(((move) & 0x1F80) >> 7)

// clang-format off
enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK, OFFBOARD };
enum { bbWhite, bbBlack, bbPawn, bbKnight, bbBishop, bbRook, bbQueen, bbKing, bbAny, bbLength };
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE };
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE };
enum { WHITE, BLACK, BOTH, NONE };
enum {
  A1 = 21, A2, A3, A4, A5, A6, A7, A8,
  B1 = 31, B2, B3, B4, B5, B6, B7, B8,
  C1 = 41, C2, C3, C4, C5, C6, C7, C8,
  D1 = 51, D2, D3, D4, D5, D6, D7, D8,
  E1 = 61, E2, E3, E4, E5, E6, E7, E8,
  F1 = 71, F2, F3, F4, F5, F6, F7, F8,
  G1 = 81, G2, G3, G4, G5, G6, G7, G8,
  H1 = 91, H2, H3, H4, H5, H6, H7, H8,
};
enum {NW=-9, N=1, NE=11, E=10, SE=9, S=-1, SW=-11, W=-10};
// clang-format on

enum {
    MOVE_QUIET = 0,
    MOVE_DOUBLEPAWN,
    MOVE_KINGCASTLE,
    MOVE_QUEENCASTLE,
    MOVE_CAPTURE,
    MOVE_EPCAPTURE,
    MOVE_KNIGHTPROMOTE = 8,
    MOVE_BISHOPPROMOTE,
    MOVE_ROOKPROMOTE,
    MOVE_QUEENPROMOTE,
    MOVE_KNIGHTPROMOTECAPTURE,
    MOVE_BISHOPPROMOTECAPTURE,
    MOVE_ROOKPROMOTECAPTURE,
    MOVE_QUEENPROMOTECAPTURE
};

enum { NO_CASTLE, WK_CASTLE, WQ_CASTLE, BK_CASTLE, BQ_CASTLE };
typedef unsigned long long ULL;

typedef struct {
    unsigned long long bitboard[bbLength];

    int kings[2];

    int turn;
    int halfmove;
    int fullmove;
    int castle;
    int enpassant;
} BOARD_STATE;

typedef struct {
    unsigned long compress;
    int type;
    int piece;
    int startSquare;
    int endSquare;
    int captured;
    int epcapture;
    int twopawnmove;
    int castled;
    int promotion;
    int priorep;
    int priorcastle;
} MOVE;

extern int epMap[120];
extern int sq120sq64Map[120];
extern int sq64sq120Map[64];
extern int colorMap[OFFBOARD + 1];
extern int notcolorMap[OFFBOARD + 1];
extern int genericMap[OFFBOARD + 1];
extern int toWhite[OFFBOARD + 1];
extern int toBlack[OFFBOARD + 1];
extern int onboardMap[120];
extern ULL knightJumps[64];
extern ULL kingJumps[64];

extern int pawnSqTable[64];
extern int knightSqTable[64];
extern int bishopSqTable[64];
extern int rookSqTable[64];
extern int queenSqTable[64];
extern int kingSqTable[64];

// init.c
extern void initBoard(BOARD_STATE *board);
extern void clearBoard(BOARD_STATE *board);
extern void init(BOARD_STATE *board);

// board.c

extern void setPiece(int piece, int file, int rank, BOARD_STATE *board);
extern void setPiece120(int piece, int sq, BOARD_STATE *board);

extern int getPieceFR(int file, int rank, BOARD_STATE *board);
extern int getPieceSq120(int sq, BOARD_STATE *board);

extern int hasEmptyEnemyPiece120(int sq, int color, BOARD_STATE *board);
extern int hasEnemyPiece120(int sq, BOARD_STATE *board);
extern int isEmptySquare(int sq, BOARD_STATE *board);

extern int getGenericPieceSq120(int sq, BOARD_STATE *board);
extern int getColorSq120(int sq, BOARD_STATE *board);

// moves.c
extern int generateMoves(BOARD_STATE *board, MOVE *move);
extern void makeMove(BOARD_STATE *board, MOVE move);
extern void unmakeMove(BOARD_STATE *board, MOVE move);
extern int isAttacked(BOARD_STATE *board, int sq, int enemycolor);
extern int isLegalMove(BOARD_STATE *board, MOVE move);

// utils.c
extern void printBits(unsigned long long num);
extern void printBitboard(unsigned long long num);
extern void printBitboardIndex64();
extern void printBitboardIndex120();
extern int countBits(unsigned long long b);
extern void printBoard(BOARD_STATE *board);
extern void printBoardIndex();
extern int bitScanForward(ULL bb);

// test.c
extern void test();

// perft.c
extern ULL perft(int depth);
extern ULL perft_rec(int depth, BOARD_STATE *board);
extern ULL perft_rec_bulk(int depth, BOARD_STATE *board);
extern void printBenchmark(int depth);

// eval.c
extern int eval(BOARD_STATE *board);
extern int negaMax(BOARD_STATE *board, int depth);
extern void printBestMove(int depth, BOARD_STATE *board);
extern MOVE makeBestMove(int depth, BOARD_STATE *board);

// uci.c
extern int loadFEN(char *fen, BOARD_STATE *board, int startIndex);
extern void startUCI();

#endif
