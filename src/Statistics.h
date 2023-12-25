#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include "State.h"

class Statistics {
 public:
    State state;
    uint32_t visits;
    uint32_t results[3];

    Statistics();
    explicit Statistics(State);
    explicit Statistics(Statistics*);
};
