#pragma once

#include "Definitions.h"
#include "Statistics.h"

typedef float FloatPrecision;

// Transposition Table
extern const uint64_t SEED;
extern std::vector<std::vector<int64_t>> zobrist_table;
extern std::unordered_map<uint64_t, Statistics*>* TT;
extern uint32_t TT_subcheck;
extern uint32_t TT_hits;

extern std::random_device rand_device;
extern std::mt19937 rng;
extern FloatPrecision logTable[MaxSimulations];