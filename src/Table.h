#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include <vector>
#include "Statistics.h"
#include "State.h"

using std::vector;


class Table {
 public:
    /**
     * Default constructor
    */
    Table();

    /**
     * Default destructor
    */
    ~Table();

    /**
     * Get statistics for a given state
     * Returns nullptr if state is not in table
    */
    Statistics* get(const State& state) const;

    /**
     * Put statistics for a given state
     * If state is already in table, it will be overwritten
    */
    void put(const State& state, Statistics* stats);

    /**
     * Reset the table
    */
    void clear();

    /**
     * Set the size of the Table
    */
    void setSize(uint32_t size);

    /**
     * How many elements the Table can hold
    */
    uint32_t getSize() const;

 private:
    /**
     * Hash the state
    */
    size_t index(const State& state) const;

    /**
     * Table size
    */
    uint32_t size;

    /**
     * Table
    */
    vector<Statistics*> table;
};
