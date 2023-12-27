/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include "Statistics.h"


Statistics::Statistics()
    : state(new State()), visits(0) {
    memset(results, 0, sizeof(uint32_t) * 3);
}

Statistics::Statistics(State state)
    : state(state), visits(0) {
    memset(results, 0, sizeof(uint32_t) * 3);
}

Statistics::Statistics(Statistics* source)
    : state(State(source->state)), visits(source->visits) {
    memcpy(results, source->results, sizeof(uint32_t) * 3);
}
