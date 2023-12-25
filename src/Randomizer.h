#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include <random>
#include <cstdint>

class Randomizer {
 private:
    static std::random_device rd;
    static std::mt19937_64 rng;

 public:
    // Initialize the random number generator with a seed
    static void initialize(uint64_t seed);

    // Get a random integer between 0 and max-1
    template <typename T>
    static T randomInt(T max);

    // Access the RNG directly
    static std::mt19937_64& getRng();
};
