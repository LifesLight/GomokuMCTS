/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include "Statistics.h"


Statistics::Statistics()
    : visits(0), state(new State()) {
    memset(results, 0, sizeof(uint32_t) * 3);
}

Statistics::Statistics(State state)
    : visits(0), state(state) {
    memset(results, 0, sizeof(uint32_t) * 3);
}
