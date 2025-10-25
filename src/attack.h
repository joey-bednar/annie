#ifndef ATTACK_H
#define ATTACK_H

#include "board.h"
#include "misc.h"

extern ULL knightJumps[64];
extern ULL kingJumps[64];
extern ULL slidingRay[8][64];

int isAttacked(BOARD_STATE *board, int sq, int enemycolor);
int isLegalMove(BOARD_STATE *board, MOVE move);
int isCheck(BOARD_STATE *board, MOVE move);

#endif
