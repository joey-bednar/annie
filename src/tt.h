#ifndef TT_H
#define TT_H

#include "board.h"
#include "misc.h"

#define TT_SIZE 10000000
#define TT_EMPTY 1234
#define TT_EMPTY_FLAG 0
#define TT_EXACT_FLAG 1
#define TT_ALPHA_FLAG 2
#define TT_BETA_FLAG 3

typedef struct {
    ULL hash;
    MOVE best;
    int depth;
    int flag;
    int val;
} TT;

extern TT *tt;

void initTT();
int probeTT(ULL hash, MOVE *best, int alpha, int beta, int depth);
void storeTT(BOARD_STATE *board, MOVE best, int val, int flag, int depth);

#endif
