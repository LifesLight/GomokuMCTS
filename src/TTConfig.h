#include "Statistics.h"
// Transposition Table
extern const uint64_t SEED;
extern std::vector<std::vector<int64_t>> zobrist_table;
extern std::unordered_map<uint64_t, Statistics*>* TT;
extern uint32_t TT_subcheck;
extern uint32_t TT_hits;