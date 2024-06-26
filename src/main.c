#include "defs.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int epMap[120];
int sq120sq64Map[120];
int sq64sq120Map[64];
int colorMap[OFFBOARD + 1];
int notcolorMap[OFFBOARD + 1];
int genericMap[OFFBOARD + 1];
int toColor[2][OFFBOARD + 1];
int onboardMap[120];
ULL knightJumps[64];
ULL kingJumps[64];

ULL zobrist_vals[12][64];
ULL zobristB2M;

int pawnOffset[2][4] = {{N, 2 * N, NW, NE}, {S, 2 * S, SW, SE}};

int inputDepth;
int inputTime[2];
int inputInc[2];

PVENTRY hashtable[PVSIZE];

int main() {
    BOARD_STATE board;
    init(&board);

    PVENTRY *hashtable = (PVENTRY *)calloc(PVSIZE, sizeof(PVENTRY));

    startUCI();

    free(hashtable);

    return 0;
}
