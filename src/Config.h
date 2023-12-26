#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */

#define BOARD_SIZE 15
#define MAX_SIMULATIONS 250000000
#define EXPLORATION_BIAS 1.4142135624

#if BOARD_SIZE < 16
typedef uint8_t index_t;
#else
typedef uint16_t index_t;
#endif
