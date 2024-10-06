#include "defs.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define INPUTLEN 10000

static int parseUCINumber(char *string, const char *leader,
                          const int defaultReturn) {
    char *depthStr = strstr(string, leader);
    if (depthStr != NULL) {
        int i = strlen(leader);
        int val = 0;
        while (depthStr[i] >= '0' && depthStr[i] <= '9') {
            val = val * 10 + (int)(depthStr[i] - '0');
            i++;
        }
        return val;
    }
    return defaultReturn;
}

static void playUCIMove(BOARD_STATE *board, int start, int end, char promo) {
    int piece;

    switch (promo) {
    case 'q':
        piece = bbQueen;
        break;
    case 'r':
        piece = bbRook;
        break;
    case 'b':
        piece = bbBishop;
        break;
    case 'n':
        piece = bbKnight;
        break;
    default:
        piece = '\0';
    }

    MOVE moves[MAX_LEGAL_MOVES];
    int n_moves = generateMoves(board, moves);

    for (int i = 0; i < n_moves; ++i) {
        if (START120(moves[i]) == start && END120(moves[i]) == end &&
            isLegalMove(board, moves[i])) {
            if (promo == ' ' || promo == '\n') {
                makeMove(board, moves[i]);
                return;
            } else if (PROMOTED(moves[i]) == piece) {
                makeMove(board, moves[i]);
                return;
            }
        }
    }
}

static void parseMoves(char *string, BOARD_STATE *board) {

    // find and shift to "moves" occurrence in input
    int i = 7;
    char *ret = strstr(string, " moves ");
    if (ret == NULL) {
        return;
    }
    string = ret;

    while (string[i] != '\n' && string[i] != '\0') {
        if (string[i] == ' ') {
            ++i;
        } else {

            int startSq = CHAR2SQ120(string[i], string[i + 1]);
            int endSq = CHAR2SQ120(string[i + 2], string[i + 3]);

            char promo = string[i + 4];

            playUCIMove(board, startSq, endSq, promo);

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
            ++index;
        }
        ++i;
    }

    // set turn
    ++i;
    if (fen[i] == 'w') {
        board->turn = WHITE;
    } else if (fen[i] == 'b') {
        board->turn = BLACK;
    }

    // set castling ability
    board->castle = 0;
    i = i + 2;
    while (fen[i] != ' ') {
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
        ++i;
    }

    // set en passant
    ++i;
    board->enpassant = OFFBOARD;
    int rank = 0;
    int file = 0;
    while (fen[i] != ' ') {

        if (fen[i] >= 'a' && fen[i] <= 'h') {
            file = (int)(fen[i] - 'a');
        } else if (fen[i] >= '1' && fen[i] <= '8') {
            rank = (int)(fen[i] - '1');
            board->enpassant = FR2SQ120(file, rank);
        }
        ++i;
    }

    while (fen[i] == ' ') {
        ++i;
    }

    // set half move counter
    int halfmove = 0;
    while (fen[i] != ' ') {
        if (fen[i] >= '0' && fen[i] <= '9') {
            halfmove = halfmove * 10;
            halfmove = halfmove + (int)(fen[i] - '0');
        }
        ++i;
    }
    board->halfmove = halfmove;

    while (fen[i] == ' ') {
        ++i;
    }

    // set full move counter
    int fullmove = 0;
    while (fen[i] != ' ' && fen[i] != '\n') {
        if (fen[i] >= '0' && fen[i] <= '9') {
            fullmove = fullmove * 10;
            fullmove = fullmove + (int)(fen[i] - '0');
        }
        ++i;
    }
    board->fullmove = fullmove;

    loadZobrist(board);

    return i;
}

static void parseSearchParameters(char *input) {
    // parse inputs
    inputDepth = parseUCINumber(input, " depth ", MAX_DEPTH);
    inputTime[WHITE] = parseUCINumber(input, " wtime ", DEFAULT_TIME);
    inputTime[BLACK] = parseUCINumber(input, " btime ", DEFAULT_TIME);
    inputInc[WHITE] = parseUCINumber(input, " winc ", DEFAULT_INC);
    inputInc[BLACK] = parseUCINumber(input, " binc ", DEFAULT_INC);
    inputMovetime = parseUCINumber(input, " movetime ", 0);
}

void *searchThread(void *args) {

    BOARD_STATE *board = (BOARD_STATE *)args;
    search(board);
    fflush(stdout);
    return NULL;
}

void startUCI() {
    BOARD_STATE board;
    init(&board);
    pthread_t thread_id;

    while (TRUE) {
        char input[INPUTLEN];
        for (int i = 0; i < INPUTLEN; ++i) {
            input[i] = '\0';
        }
        fgets(input, INPUTLEN, stdin);

        if (strncmp("ucinewgame", input, 10) == 0) {
            initBoard(&board);
            printf("readyok\n");
        } else if (strncmp("uci", input, 3) == 0) {
            printf("id name %s v%s\n", NAME, VERSION);
            printf("id author %s\n", AUTHOR);
            printf("option name Ponder type check default false\n");
            printf("uciok\n");
        } else if (strcmp("isready\n", input) == 0) {
            printf("readyok\n");
        } else if (strcmp("debug\n", input) == 0) {
            printf("\n");
            printBoard(&board);
            printf("\n");
        } else if (strncmp("position startpos", input, 17) == 0) {
            initBoard(&board);
            parseMoves(input, &board);
        } else if (strncmp("position fen ", input, 13) == 0) {
            initBoard(&board);
            loadFEN(input, &board, 13);
            parseMoves(input, &board);

        } else if (strncmp("go perft ", input, 9) == 0) {

            int depth = parseUCINumber(input, " perft ", 0);
            perft(depth, &board);

        } else if (strncmp("go ponder\n", input, 9) == 0) {

            // stop searches, wait for thread to join
            board.stopped = TRUE;
            pthread_join(thread_id, NULL);
            // pthread_exit(&thread_id);

            // parse inputs
            parseSearchParameters(input);

            // create ponder thread
            board.ponder = TRUE;
            pthread_create(&thread_id, NULL, searchThread, (void *)&board);

        } else if (strncmp("go\n", input, 2) == 0) {

            // stop searches, wait for thread to join
            board.stopped = TRUE;
            pthread_join(thread_id, NULL);
            // pthread_exit(&thread_id);

            // parse inputs
            parseSearchParameters(input);

            // create search thread
            // board.stopped = FALSE;
            board.ponder = FALSE;
            pthread_create(&thread_id, NULL, searchThread, (void *)&board);

        } else if (strcmp("ponderhit\n", input) == 0) {
            // stop searches, wait for thread to join
            // board.stopped = TRUE;
            // pthread_join(thread_id, NULL);
            // pthread_exit(&thread_id);

            // create search thread
            // board.stopped = FALSE;
            board.ponder = FALSE;
            board.cutoffTime = setCutoff(&board);
            board.start = clock();
            // pthread_create(&thread_id, NULL, searchThread, (void *)&board);
        } else if (strcmp("stop\n", input) == 0) {
            board.stopped = TRUE;
            pthread_join(thread_id, NULL);
            // pthread_exit(&thread_id);
        } else if (strcmp("quit\n", input) == 0) {
            board.stopped = TRUE;
            pthread_join(thread_id, NULL);
            // pthread_exit(&thread_id);
            break;
        } else if (strcmp("setoption\n", input) == 0) {
            // incomplete
        }
        fflush(stdout);
    }
}
