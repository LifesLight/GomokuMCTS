#pragma once

#include "Config.h"
#include "TTConfig.h"

// Optimized Gomoku game state interface for MCTS

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

#if BoardSize > 32
typedef int64_t BLOCK;
#elif BoardSize > 16
typedef int32_t BLOCK;
#elif BoardSize > 8
typedef int16_t BLOCK;
#else
typedef int8_t BLOCK;
#endif

class State
{
public:
    // Mask
    BLOCK m_array[BoardSize * 6];
    // Color
    BLOCK c_array[BoardSize * 6];
    // Last is last played move, empty is remaining empty fields
    uint16_t last, empty;
    // 0:p0win 1:p1win 2:none
    uint8_t result;
    uint64_t hash_value;

    State();
    State(State*);

    // Make move
    void action(uint16_t);
    // Get list of remaining empty fields as indecies
    std::vector<uint16_t> possible();
    // Is terminal game state
    bool terminal();
    // String representation of state
    std::string toString();

private:
    bool check_for_5();
    uint64_t hash();
};