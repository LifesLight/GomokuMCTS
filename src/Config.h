#pragma once

#include "iostream"
#include "vector"
#include "list"
#include "random"
#include "algorithm"
#include "string"
#include "chrono"
#include "cstring"
#include "unordered_map"

#define BoardSize 15
#define ExplorationBias 1.41421356237
#define MaxSimulations 1'000'000
typedef float FloatPrecision;

extern std::random_device rand_device;
extern std::mt19937 rng;
extern FloatPrecision logTable[MaxSimulations];

