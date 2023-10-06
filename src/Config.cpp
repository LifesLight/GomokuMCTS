#include "Config.h"

std::vector<std::vector<int64_t> > zobrist_table(BoardSize * BoardSize, std::vector<int64_t>(3));
std::unordered_map<uint64_t, Statistics*>* TT = new std::unordered_map<uint64_t, Statistics*>;
uint32_t TT_hits = 0;

std::random_device rand_device;
std::mt19937 rng(rand_device());
FloatPrecision logTable[MaxSimulations];