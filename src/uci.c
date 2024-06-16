#include "defs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define INPUTLEN 1000
#define POSSTARTLEN 17
#define POSFENLEN 13

#define DEFAULTDEPTH 5

static void playUCIMove(BOARD_STATE *board, int start, int end, char promo) {
    int piece;
    int pieceCap;

    switch (promo) {
    case 'q':
        piece = MOVE_QUEENPROMOTE;
        pieceCap = MOVE_QUEENPROMOTECAPTURE;
        break;
    case 'r':
        piece = MOVE_ROOKPROMOTE;
        pieceCap = MOVE_ROOKPROMOTECAPTURE;
        break;
    case 'b':
        piece = MOVE_BISHOPPROMOTE;
        pieceCap = MOVE_BISHOPPROMOTECAPTURE;
        break;
    case 'n':
        piece = MOVE_KNIGHTPROMOTE;
        pieceCap = MOVE_KNIGHTPROMOTECAPTURE;
        break;
    default:
        break;
    }

    // printf("playUCIMove %d %d\n",start,end);

    MOVE moves[MAX_LEGAL_MOVES];
    int n_moves = generateMoves(board, moves);

    for (int i = 0; i < n_moves; i++) {
        if (SQ64SQ120(START(moves[i].compress)) == start &&
            SQ64SQ120(END(moves[i].compress)) == end &&
            isLegalMove(board, moves[i])) {
            if (promo == ' ' || promo == '\n') {
                makeMove(board, moves[i]);
                // printf("move no promote\n");
                return;
            } else if (moves[i].type == piece || moves[i].type == pieceCap) {
                makeMove(board, moves[i]);
                // printf("move promote type %d\n",moves[i].type);
                return;
            }
        }
    }
}

static void parseMoves(char *string, BOARD_STATE *board, int startIndex) {
    int i = startIndex;

    while (string[i] != '\n' && string[i] != '\0') {
        if (string[i] == ' ') {
            i++;
        } else {

            // printf("string[%d]: %c\n",i,string[i]);
            int startSq = CHAR2SQ120(string[i], string[i + 1]);
            int endSq = CHAR2SQ120(string[i + 2], string[i + 3]);

            char promo = string[i + 4];

            // printf("parseMoves: %d %d %c\n",startSq,endSq,promo);

            // printBoard(board);
            playUCIMove(board, startSq, endSq, promo);
            // printBoard(board);

            if (promo == ' ') {
                i = i + 4;
            } else if (promo == '\n') {
                break;
            } else {
                i = i + 5;
            }
        }
    }
}

int loadFEN(char *fen, BOARD_STATE *board, int startIndex) {

    clearBoard(board);

    // load all pieces
    int i = startIndex;
    int index = 0;
    while (fen[i] != ' ') {
        if (fen[i] >= '0' && fen[i] <= '8') {
            int val = (int)(fen[i] - '0');
            index = index + val;
        } else if (fen[i] != '/' && fen[i] != ' ') {
            int file = index % 8;
            int rank = 7 - (index / 8);
            int sq = FR2SQ120(file, rank);
            switch (fen[i]) {
            case 'p':
                setPiece120(bP, sq, board);
                break;
            case 'r':
                setPiece120(bR, sq, board);
                break;
            case 'n':
                setPiece120(bN, sq, board);
                break;
            case 'b':
                setPiece120(bB, sq, board);
                break;
            case 'q':
                setPiece120(bQ, sq, board);
                break;
            case 'k':
                setPiece120(bK, sq, board);
                break;

            case 'P':
                setPiece120(wP, sq, board);
                break;
            case 'R':
                setPiece120(wR, sq, board);
                break;
            case 'N':
                setPiece120(wN, sq, board);
                break;
            case 'B':
                setPiece120(wB, sq, board);
                break;
            case 'Q':
                setPiece120(wQ, sq, board);
                break;
            case 'K':
                setPiece120(wK, sq, board);
                break;
            }
            index++;
        }
        i++;
    }

    // set turn
    i++;
    if (fen[i] == 'w') {
        board->turn = WHITE;
    } else if (fen[i] == 'b') {
        board->turn = BLACK;
    }

    // set castling ability
    board->castle = 0;
    i = i + 2;
    while (fen[i] != ' ' && fen[i] != '-') {
        switch (fen[i]) {
        case 'K':
            SETBIT(board->castle, WK_CASTLE);
            break;
        case 'Q':
            SETBIT(board->castle, WQ_CASTLE);
            break;
        case 'k':
            SETBIT(board->castle, BK_CASTLE);
            break;
        case 'q':
            SETBIT(board->castle, BQ_CASTLE);
            break;
        }
        i++;
    }

    // set en passant
    i++;
    board->enpassant = OFFBOARD;
    int rank = 0;
    int file = 0;
    while (fen[i] != ' ' && fen[i] != '-') {

        if (fen[i] >= 'a' && fen[i] <= 'h') {
            file = (int)(fen[i] - 'a');
        } else if (fen[i] >= '1' && fen[i] <= '8') {
            rank = (int)(fen[i] - '1');
            board->enpassant = FR2SQ120(file, rank);
        }
        i++;
    }

    // set full move counter
    i = i + 2;
    int halfmove = 0;
    while (fen[i] != ' ') {
        if (fen[i] >= '0' && fen[i] <= '9') {
            // printf("partial halfmove: %d\n",(int)(fen[i]-'0'));
            halfmove = halfmove * 10;
            halfmove = halfmove + (int)(fen[i] - '0');
        }
        i++;
    }
    board->halfmove = halfmove;
    // printf("half move: %d\n", halfmove);

    // set half move counter
    i = i + 1;
    int fullmove = 0;
    while (fen[i] != ' ' && fen[i] != '\n') {
        if (fen[i] >= '0' && fen[i] <= '9') {
            // printf("partial fullmove: %d\n",(int)(fen[i]-'0'));
            fullmove = fullmove * 10;
            fullmove = fullmove + (int)(fen[i] - '0');
        }
        i++;
    }
    board->fullmove = fullmove;

    return i + 9;
}

void startUCI() {
    BOARD_STATE board;
    initBoard(&board);

    while (TRUE) {
        char input[INPUTLEN];
        for (int i = 0; i < INPUTLEN; i++) {
            input[i] = '\0';
        }
        fgets(input, INPUTLEN, stdin);
        if (strcmp("ucinewgame", input) == 0) {
            printf("readyok\n");
            initBoard(&board);
        } else if (strncmp("uci", input, 3) == 0) {
            printf("id name Annie Edison\n");
            printf("id author Joey Bednar\n");
            printf("uciok\n");
        } else if (strcmp("isready\n", input) == 0) {
            printf("readyok\n");
        } else if (strcmp("position startpos\n", input) == 0) {
            initBoard(&board);
        } else if (strncmp("position startpos \n", input, POSSTARTLEN) == 0) {
            initBoard(&board);
            parseMoves(input, &board, POSSTARTLEN);
        } else if (strncmp("position fen ", input, POSFENLEN) == 0) {
            initBoard(&board);
            int i = loadFEN(input, &board, POSFENLEN);
            parseMoves(input, &board, i);
        } else if (strncmp("go\n", input, 2) == 0) {
            printBestMove(DEFAULTDEPTH, &board);
        } else if (strcmp("stop\n", input) == 0) {
            // printf("stop\n");
        } else if (strcmp("quit\n", input) == 0) {
            break;
        }
        // printBoard(&board);
        fflush(stdout);
    }
}
