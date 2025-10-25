#ifndef UCI_H
#define UCI_H

#include "board.h"
#include "misc.h"

// name
#define NAME "Amelia"
#define VERSION "0.15"
#define AUTHOR "Joey Bednar"

#define CHAR2FILE(c) ((int)((c) - 'a'))
#define CHAR2RANK(c) ((int)((c) - '1'))
#define CHAR2SQ120(f, r) (FR2SQ120((CHAR2FILE((f))), (CHAR2RANK((r)))))

extern int inputDepth;
extern int inputMovetime;
extern int inputTime[2];
extern int inputInc[2];

int loadFEN(char *fen, BOARD_STATE *board, int startIndex);
void startUCI();

#endif
