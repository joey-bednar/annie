#include "defs.h"
#include <assert.h>

// clear table
void initTT() {
    for (int i = 0; i < TT_SIZE; ++i) {
        tt[i].hash = 0ull;
        tt[i].depth = 0;
        tt[i].flag = TT_EMPTY_FLAG;
        tt[i].best = 0ull;
        tt[i].val = 0;

        pvTable[i].hash = 0ull;
        pvTable[i].depth = 0;
        pvTable[i].flag = TT_EMPTY_FLAG;
        pvTable[i].best = 0ull;
        pvTable[i].val = 0;
    }
}

int probeTT(ULL hash, MOVE *best, int alpha, int beta, int depth) {
    TT *entry = &tt[hash % TT_SIZE];

    if (entry->hash != hash) {
        return TT_EMPTY;
    }

    if (entry->flag == TT_EMPTY_FLAG) {
        return TT_EMPTY;
    }

    if (entry->depth < depth) {
        return TT_EMPTY;
    }

    if (entry->flag == TT_EXACT_FLAG) {
        *best = entry->best;
        return entry->val;
    } else if (entry->flag == TT_ALPHA_FLAG && entry->val <= alpha) {
        return alpha;
    } else if (entry->flag == TT_BETA_FLAG && entry->val >= beta) {
        return beta;
    }

    return TT_EMPTY;
}

MOVE pvTT(ULL hash) {
    TT *entry = &pvTable[hash % TT_SIZE];

    if (entry->hash == hash && entry->flag == TT_EXACT_FLAG) {
        return entry->best;
    }

    return 0ull;
}

MOVE moveTT(ULL hash) {
    TT *entry = &tt[hash % TT_SIZE];

    if (entry->hash == hash) {
        return entry->best;
    }

    return 0ull;
}

void storePVTT(ULL hash, MOVE best, int val, int flag, int depth) {

    int index = hash % TT_SIZE;
    if (pvTable[index].hash == hash && pvTable[index].depth > depth) {
        return;
    }

    pvTable[index].hash = hash;
    pvTable[index].depth = depth;
    pvTable[index].flag = flag;
    pvTable[index].best = best;
    pvTable[index].val = val;
}

void storeTT(ULL hash, MOVE best, int val, int flag, int depth) {

    int index = hash % TT_SIZE;

    if (hash == tt[index].hash && depth < tt[index].depth) {
        return;
    }

    tt[index].hash = hash;

    tt[index].depth = depth;
    tt[index].flag = flag;
    tt[index].best = best;
    tt[index].val = val;
}
