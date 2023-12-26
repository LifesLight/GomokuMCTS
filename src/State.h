#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */

#include <stdint.h>
#include <vector>
#include <string>
#include <cstring>
#include <random>
#include <sstream>

#include "Config.h"
#include "Randomizer.h"
#include "Utilities.h"

using std::vector;
using std::string;
using std::to_string;
using std::stringstream;
using std::endl;

/* Partitions :
-----------------------------------------------------------
1: Horizontal   2: Vertical     3: LDiagonal    4: RDiagonal
xxxx            xxxo            oooo            oooo
xoxx            xoxo            xooo            oooo
xxoo            xxoo            xxoo            xooo
oooo            xxoo            xooo            xxxo
                                xxoo            xoxo
                                xxoo            xxoo
                                xooo            xooo
----------------------------------------------------------- */


/**
 * Bitmask for stone / no stone
*/
#if BOARD_SIZE > 32
typedef int64_t block_t;
#elif BOARD_SIZE > 16
typedef int32_t block_t;
#elif BOARD_SIZE > 8
typedef int16_t block_t;
#else
typedef int8_t block_t;
#endif


/**
 * Game state
 * Speed optimized for MCTS
*/
class State {
 public:
    /**
    * Default constructor
    */
    State();

    /**
     * Copy constructor
    */
    explicit State(State* source);

    /**
     * Make action via x, y on state
    */
    void action(uint8_t x, uint8_t y);

    /**
     * Make action on state
    */
    void action(uint16_t action);

    /**
     * Get list of remaining empty fields as indicies
    */
    vector<uint16_t> possible();

    /**
     * Check if state is terminal
    */
    bool terminal();

    /**
    * Convert state to string
    */
    string toString();

    /**
     * Init zobrist hash table
    */
    static void init_zobrist();

    /**
     * Get the result
    */
    uint8_t get_result();

    /**
     * Get the last move
    */
    uint16_t get_last();

    /**
     * Get the empty fields
    */
    uint16_t get_empty();

    /**
     * Get the hash value
    */
    uint64_t get_hash();

    /**
     * Is cell empty
    */
    bool is_empty(uint16_t index);

    /**
     * Is cell empty
    */
    bool is_empty(uint8_t x, uint8_t y);

    /**
     * Get the color of cell
    */
    int8_t getCellValue(uint16_t index);

    /**
     * Get the color of cell
    */
    int8_t getCellValue(uint8_t x, uint8_t y);

 private:
    /**
     * Check for 5-Stone alignment
    */
    bool check_for_5();

    /**
    * Calculate inital hash value
    */
    uint64_t hash();

    /**
     * Bitmask for stone / no stone
    */
    block_t m_array[BOARD_SIZE * 6];

    /**
     * Bitmask for color
    */
    block_t c_array[BOARD_SIZE * 6];

    /**
    * Last move
    */
    uint16_t last;

    /**
     * How many empty fields are left
    */
    uint16_t empty;

    /**
     * Result of game
     * 0: p0win 1: p1win 2: none
     */
    uint8_t result;

    /**
    * Zobrist hash value
    */
    uint64_t hash_value;

    /**
     * Zobrist hash table
    */
    static vector<vector<int64_t>> zobrist_table;
};
