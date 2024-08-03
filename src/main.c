#include "defs.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

TT pvTable[TT_SIZE];
TT tt[TT_SIZE];

int main() {
    BOARD_STATE board;
    init(&board);

    TT *pvTable = (TT *)calloc(TT_SIZE, sizeof(TT));

    TT *tt = (TT *)calloc(TT_SIZE, sizeof(TT));

    // int p;
    // p = probeTT(board.hash, 0, 0, 0);
    // printf("%d\n",p);
    //
    // writeTTExact(board.hash, 20, 0);
    //
    // p = probeTT(board.hash, 0, 0, 0);
    // printf("%d\n",p);

    startUCI();

    free(pvTable);
    free(tt);

    return 0;
}
