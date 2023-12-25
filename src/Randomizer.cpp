/**
 * Copyright (c) Alexander Kurtz 2023
 */

#include "Randomizer.h"

std::random_device Randomizer::rd;
std::mt19937_64 Randomizer::rng(rd());

void Randomizer::initialize(uint64_t seed) {
    rng.seed(seed);
}

template <typename T>
T Randomizer::randomInt(T max) {
    std::uniform_int_distribution<T> dist(0, max - 1);
    return dist(rng);
}

std::mt19937_64& Randomizer::getRng() {
    return rng;
}

// Explicit template instantiation for common types
template uint8_t Randomizer::randomInt<uint8_t>(uint8_t);
template uint16_t Randomizer::randomInt<uint16_t>(uint16_t);
template uint32_t Randomizer::randomInt<uint32_t>(uint32_t);
template uint64_t Randomizer::randomInt<uint64_t>(uint64_t);
