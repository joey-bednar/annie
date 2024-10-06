#include "defs.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    BOARD_STATE board;

    init(&board);

    startUCI();

    free(tt);

    return 0;
}
