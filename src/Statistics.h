#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include "Config.h"
#include "State.h"


/**
 * Cross Node state data
 * Used to sync data between nodes which have the same state
*/
struct Statistics {
    /**
     * Current state
    */
    State state;

    /**
     * Number of visits
    */
    uint32_t visits;

    /**
     * Inverse number of visits
     * Used to calculate UCT
    */
    #if SPEED_LEVEL >= 2
    double inverseVisits;
    #endif

    /**
     * Results of simulations
     * 0: p0win 1: p1win 2: draws
    */
    uint32_t results[3];

    Statistics();
    explicit Statistics(State);
};
