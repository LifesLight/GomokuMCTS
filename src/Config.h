#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */

#define BOARD_SIZE 15
#define MAX_SIMULATIONS 250000000
#define EXPLORATION_BIAS 1.4142135624

/**
 * The higher the speed level, the faster the program runs
 * Also the more memory it uses
 * 
 * 0: Not done, memory efficient State class
 * 1: Fast State class
 * 2: Cache inverse visits (Slower right now?)
*/
#define SPEED_LEVEL 1


/**
 * Type to 1D index the board
 * Save memory if [BOARD_SIZE ^ 2] < [2 ^ 16]
*/
#include <stdint.h>

#if BOARD_SIZE < 16
typedef uint8_t index_t;
#else
typedef uint16_t index_t;
#endif
