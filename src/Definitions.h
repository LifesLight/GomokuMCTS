#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <random>
#include <algorithm>
#include <string>
#include <chrono>
#include <cstring>
#include <unordered_map>
#include <sstream>
#include <iomanip>

#define BoardSize 15
// MCTS exploration value (higher is broader search)
#define ExplorationBias 1.41421356237
// Set this to the max amount of simulations in one simulation pass
#define MaxSimulations 10'000'000