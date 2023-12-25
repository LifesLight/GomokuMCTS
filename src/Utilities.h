#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */

#include <stdint.h>

#include "Config.h"

class Utils {
 public:
    template <typename T>
    static void indexToCords(const uint16_t index, T *x, T *y) {
        *x = index % BOARD_SIZE;
        *y = index / BOARD_SIZE;
    }


    template <typename T>
    static void cordsToIndex(uint16_t *index, const T x, const T y) {
        (*index) = y * BOARD_SIZE + x;
    }
};
