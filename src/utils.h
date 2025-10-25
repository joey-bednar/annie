#ifndef UTILS_H
#define UTILS_H

#include "board.h"
#include "misc.h"

void printBits(unsigned long long num);
void printBitboard(unsigned long long num);
void printBitboardIndex64();
void printBitboardIndex120();
int countBits(unsigned long long b);
void printBoard(BOARD_STATE *board);
void printBoardIndex();
int bitScanForward(ULL bb);
int bitScanReverse(ULL bb);
int manhattanDistance(int sq1, int sq2);

#endif
