#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */


/**
 * Board size
 * Must be between 4 < BOARD_SIZE < 64
*/
#define BOARD_SIZE 13

/**
 * Hard limit of simulations per move
 * If reached, the MCTS will stop and return the best move
 * Needed to prevent overflow of the log table and other optimizations
*/
#define MAX_SIMULATIONS 10'000'000

/**
 * Exploration bias
 * Higher values will favor exploration
 * Lower values will favor exploitation
*/
#define EXPLORATION_BIAS 1.4142135624

/**
 * Type to 1D index the board
 * Save memory if [BOARD_SIZE ^ 2] < [2 ^ 16]
*/
#if BOARD_SIZE < 16
typedef uint8_t index_t;
#else
typedef uint16_t index_t;
#endif
