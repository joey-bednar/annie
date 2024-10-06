#ifndef DEFS_H

// name
#define NAME "Amelia"
#define VERSION "0.15"
#define AUTHOR "Joey Bednar"

// settings
#define DEFAULTDEPTH 6
#define MAX_DEPTH 100
#define QMAXDEPTH 100
#define DEFAULT_TIME 3600000
#define DEFAULT_INC 0
#define ASPIRATION_WINDOW 50

// internal constants
#define FALSE 0
#define TRUE 1
#define MAX_LEGAL_MOVES 256
#define MAX_GAME_LENGTH 512
#define MATE 50000
#define MATETHRESHOLD 50
#define INF 99999

// squares
#define SQ120R(sq) (((sq) % 10) - 1)
#define SQ120F(sq) (((sq) - 21) / (10))
#define FR2SQ120(f, r) ((21 + (r)) + ((f) * 10))
#define FR2SQ64(f, r) ((r) + ((f) * 8))
#define SQ120SQ64(sq) (sq120sq64Map[(sq)])
#define SQ64SQ120(sq) (sq64sq120Map[(sq)])
#define CHAR2FILE(c) ((int)((c) - 'a'))
#define CHAR2RANK(c) ((int)((c) - '1'))
#define CHAR2SQ120(f, r) (FR2SQ120((CHAR2FILE((f))), (CHAR2RANK((r)))))

// pieces and colors
#define COLOR(p) (colorMap[(p)])
#define NOTCOLOR(p) (notcolorMap[(p)])
#define GENERIC(p) (genericMap[(p)])
#define TOWHITE(p) (toColor[WHITE][(p)])
#define TOBLACK(p) (toColor[BLACK][(p)])
#define TOCOLOR(c, p) (toColor[c][(p)])
#define ONBOARD(sq) (onboardMap[(sq)])

// bit operations
#define CHECKBIT(bb, sq64) (((bb)) >> ((sq64)) & 1ULL)
#define CLEARBIT(bb, sq64) ((bb) &= ~(1ULL << sq64))
#define SETBIT(bb, sq64) ((bb) |= (1ULL << sq64))
#define CLEARBITBOARD(bb) ((bb) = 0)
#define BBROTATE(i) ((((i) >> 3) | ((i) << 3)) & 63) ^ 7

// math
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// attack bitboards
#define KNIGHTBB(sq64) (knightJumps[(sq64)])
#define KINGBB(sq64) (kingJumps[(sq64)])

// piece offsets and promotions
#define PAWNOFFSET(c, i) (pawnOffset[(c)][(i)])
#define ROOKOFFSET(i) (rookOffset[(i)])
#define ROOKOFFSETS rookOffset
#define BISHOPOFFSET(i) (bishopOffset[(i)])
#define BISHOPOFFSETS bishopOffset
#define PROMOTES promoteTo
#define MVVLVA(p, c) (MVVLVA[(p)][(c)])
#define PASSEDPAWN(sq64, c) (passedPawnTable[(sq64)][(c)])

// move bitmasks
#define START(move) (int)((move) & 0x3Ful)
#define START120(move) SQ64SQ120(START(move))
#define END(move) (int)(((move) & 0x0FC0) >> 6)
#define END120(move) SQ64SQ120(END(move))
#define CAPTURED(move) (int)(((move) & 0x07000ul) >> 12)
#define PROMOTED(move) (int)(((move) & 0x38000ul) >> 15)
#define EPFLAG(move) (int)(((move) & 0x40000ul) >> 18)
#define TWOPAWNFLAG(move) (int)(((move) & 0x80000ul) >> 19)
#define CASTLEFLAG(move) (int)(((move) & 0x100000ul) >> 20)
#define PIECE(move) (int)(((move) & 0x00E00000ul) >> 21)

// hash table
#define TT_SIZE 10000000
#define TT_EMPTY 1234
#define TT_EMPTY_FLAG 0
#define TT_EXACT_FLAG 1
#define TT_ALPHA_FLAG 2
#define TT_BETA_FLAG 3

#define BITLOOP(bb) for (; (bb); (bb) &= ((bb) - 1))

// clang-format off
enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK, OFFBOARD };
enum { bbWhite, bbBlack, bbPawn, bbKnight, bbBishop, bbRook, bbQueen, bbKing, bbLength };
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

enum { NO_CASTLE, WK_CASTLE, WQ_CASTLE, BK_CASTLE, BQ_CASTLE, CASTLE_LENGTH };

typedef unsigned long long ULL;
typedef unsigned long long MOVE;

typedef struct tt {
    ULL hash;
    MOVE best;
    int depth;
    int flag;
    int val;
} TT;

typedef struct {
    ULL hash;
    int castle;
    int enpassant;
    int halfmove;
} POSRECORD;

typedef struct {
    ULL bitboard[bbLength];

    int turn;
    int halfmove;
    int fullmove;
    int castle;
    int enpassant;

    ULL hash;

    MOVE killers[MAX_DEPTH][2];
    int history[2][64][64];

    int ply;

    POSRECORD playedmoves[MAX_GAME_LENGTH * 2];
    int pmindex;

    long nodes;
    int stopped;
    int cutoffTime;
    float start;

} BOARD_STATE;

extern TT *tt;
extern int ttNumIndices;

extern int inputDepth;
extern int inputMovetime;
extern int inputTime[2];
extern int inputInc[2];
extern int inputHash;

extern int epMap[120];
extern int sq120sq64Map[120];
extern int sq64sq120Map[64];
extern int colorMap[OFFBOARD + 1];
extern int notcolorMap[OFFBOARD + 1];
extern int genericMap[OFFBOARD + 1];

extern int toColor[2][OFFBOARD + 1];

extern int onboardMap[120];
extern ULL knightJumps[64];
extern ULL kingJumps[64];
extern ULL slidingRay[8][64];

extern int pawnSqTable[2][64];
extern int knightSqTable[2][64];
extern int bishopSqTable[2][64];
extern int rookSqTable[2][64];
extern int queenSqTable[2][64];
extern int kingSqTable[2][64];

extern ULL passedPawnTable[64][2];
extern ULL isolatedPawns[8];
extern int MVVLVA[8][8];

extern ULL zobrist_vals[12][64];
extern ULL zobristB2M;
extern ULL zobristEP[120];
extern ULL zobristC[64];

extern int pawnOffset[2][4];
extern const int rookOffset[4];
extern const int bishopOffset[4];
extern const int promoteTo[4];

// hash.c
extern void initZobrist();
extern void loadZobrist(BOARD_STATE *board);
extern void updateZobrist(int sq64, int piece, BOARD_STATE *board);
extern void turnZobrist(BOARD_STATE *board);
extern void updateZobristEp(int start, int end, BOARD_STATE *board);
extern void updateZobristCastle(int start, int end, BOARD_STATE *board);

// init.c
extern void initBoard(BOARD_STATE *board);
extern void clearBoard(BOARD_STATE *board);
extern void init(BOARD_STATE *board);
extern void initHistoryHeuristic(BOARD_STATE *board);

// board.c

extern int getKingSq(BOARD_STATE *board, int color);

extern void setPiece(int piece, int file, int rank, BOARD_STATE *board);
extern void setPiece120(int piece, int sq, BOARD_STATE *board);

extern int getPieceSq120(int sq, BOARD_STATE *board);
extern int getGenericPieceSq120(int sq, BOARD_STATE *board);

extern int isEmptySquare(int sq, BOARD_STATE *board);

extern int getColorSq120(int sq, BOARD_STATE *board);

extern void clearPiece(BOARD_STATE *board, int piece, int sq);
extern void placePiece(BOARD_STATE *board, int piece, int sq);
extern void updateCastling(BOARD_STATE *board, MOVE move);

// moves.c
extern int generateMoves(BOARD_STATE *board, MOVE *move);
extern void makeMove(BOARD_STATE *board, MOVE move);
extern void unmakeMove(BOARD_STATE *board, MOVE move);
extern void makeNullMove(BOARD_STATE *board);
extern void unmakeNullMove(BOARD_STATE *board);

// attack.c
extern int isAttacked(BOARD_STATE *board, int sq, int enemycolor);
extern int isLegalMove(BOARD_STATE *board, MOVE move);
extern int isCheck(BOARD_STATE *board, MOVE move);

// utils.c
extern void printBits(unsigned long long num);
extern void printBitboard(unsigned long long num);
extern void printBitboardIndex64();
extern void printBitboardIndex120();
extern int countBits(unsigned long long b);
extern void printBoard(BOARD_STATE *board);
extern void printBoardIndex();
extern int bitScanForward(ULL bb);
extern int bitScanReverse(ULL bb);
extern int manhattanDistance(int sq1, int sq2);

// perft.c
extern void perft(int depth, BOARD_STATE *board);

// eval.c
extern int isThreeFold(BOARD_STATE *board);
extern int isTwoFold(BOARD_STATE *board);
extern int isInsufficientMaterial(BOARD_STATE *board);
extern int eval(BOARD_STATE *board);
extern int isRepeatPosition(BOARD_STATE *board);
extern int getPhase(BOARD_STATE *board);

// search.c
extern void printMoveText(MOVE move);
extern int compareMoves(const void *a, const void *b);
extern void search(BOARD_STATE *board);

// uci.c
extern int loadFEN(char *fen, BOARD_STATE *board, int startIndex);
extern void startUCI();

// tt.c
extern void initTT();
extern int probeTT(ULL hash, MOVE *best, int alpha, int beta, int depth);
extern void storeTT(BOARD_STATE *board, MOVE best, int val, int flag,
                    int depth);
extern void clearTT();

#endif
