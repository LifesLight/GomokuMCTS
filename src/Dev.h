#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */

#include <stdint.h>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include "Node.h"
#include "Config.h"
#include "Utilities.h"

using std::string;
using std::vector;
using std::cout;
using std::setfill;
using std::setw;
using std::ostringstream;
using std::endl;
using std::fixed;
using std::setprecision;

class Analytics {
 public:
    static void visitsDist(Node* root);
    static void ucbDist(Node* root);
    static void overview(Node* root);

    #ifdef RAVE
    static void raveDist(Node* root);
    #endif
};
