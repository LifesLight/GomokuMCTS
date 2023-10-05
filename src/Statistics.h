#pragma once
#ifndef STATISTICS_H_
#define STATISTICS_H_

class Statistics;

#include "Definitions.h"
#include "State.h"

class Statistics
{
public:
    State state;
    uint32_t visits;
    uint32_t results[3];

    Statistics();
    Statistics(State);
    Statistics(Statistics*);
};

#endif
