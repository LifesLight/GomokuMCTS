/**
 * Copyright (c) Alexander Kurtz 2023
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <chrono> //NOLINT
#include <thread> //NOLINT

#include "Randomizer.h"
#include "Statistics.h"
#include "State.h"
#include "Config.h"
#include "Utilities.h"
#include "Node.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::chrono::system_clock;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::ostringstream;
using std::fixed;
using std::setprecision;

void init() {
    uint32_t seed = system_clock::now().time_since_epoch().count();
    Randomizer::initialize(seed);
    State::initZobrist();
    Node::initLogTable();
    Node::reserveTT(MAX_SIMULATIONS);
    Node::resetRave();
}

void human_move(State* state) {
    bool getting_input = true;
    string input_x;
    string input_y;
    uint8_t x;
    uint8_t y;
    index_t index;
    std::cout << "\n";
    while (getting_input) {
        try {
            cout << "Action X: ";
            cin >> input_x;
            cout << "Action Y: ";
            cin >> input_y;
            x = stoi(input_x);
            y = stoi(input_y);
        }
        catch (const std::exception& e) {
            std::cout << "Invalid input format!\n";
            continue;
        }

        if ((0 <= x && x < BOARD_SIZE) && (0 <= y && y < BOARD_SIZE)) {
            if (state->isEmpty(x, y))
                getting_input = false;
            else
                std::cout << "Selected field is occupied!\n";
        } else {
            std::cout << "Selection outside the field!\n";
        }
    }

    Utils::cordsToIndex(&index, x, y);
    state->action(index);
}

string evaluation(Node* best) {
    ostringstream result;

    result << "    <";
    for (index_t i = 0; i < BOARD_SIZE * 2 + 26; i++)
        result << "-";
    result << ">\n";

    int x, y;
    Utils::indexToCords(best->getParentAction(), &x, &y);

    Node* parent = best->getParent();
    const double simsInMillion = (parent->getVisits() / 1000) / 1000.0;
    const int tRelWins = parent->getScore(best->getEmpty() % 2);
    const int tRelLoss = parent->getScore(!best->getEmpty() % 2);
    const int tRelDraw = parent->getScore(2);
    const int fRelWins = best->getScore(best->getEmpty() % 2);
    const int fRelLoss = best->getScore(!best->getEmpty() % 2);
    const int fRelDraw = best->getScore(2);
    const double evaluation = best->qDelta(parent->getEmpty() % 2) /
        static_cast<double>(best->getVisits());
    const double TT_hitrate = Node::getTableHitrate();
    const double confidence = (best->getVisits() * 100) /
        static_cast<double>(parent->getVisits());
    const double draw = (best->getScore(2) * 100) /
        static_cast<double>(best->getVisits());

    result << "Action:      " << x << "," << y << "\n";
    result << "Simulations: " << simsInMillion << "M"
        << " (W:" << tRelWins
        << " L:" << tRelLoss
        << " D:" << tRelDraw << ")\n";
    result << "Evaluation:  " << fixed << setprecision(3) << evaluation
       << " (W:" << fRelWins
       << " L:" << fRelLoss
       << " D:" << fRelDraw << ")\n";
    result << "TT-Hitrate:  " << TT_hitrate << "%\n";
    result << "Confidence:  " << confidence << "%\n";
    result << "Draw:        " << draw << "%\n";

    result << "    <";
    for (index_t i = 0; i < BOARD_SIZE * 2 + 26; i++)
        result << "-";
    result << ">\n";

    return result.str();
}

void deleteTreeBackground(Node* root) {
    std::thread([root]() {
        Node::deleteTree(root);
    }).detach();
}

void MCTS_master(Node* root, State *root_state) {
    // Select best child
    Node* best = root->absBestChild();

    std::cout << evaluation(best);

    (*root_state).action(best->getParentAction());

    // Reset TT
    Node::resetRave();
    Node::resetTranspositionTable();
}

void MCTS_move(State *root_state, uint64_t simulations) {
    if (simulations > MAX_SIMULATIONS)
        throw std::invalid_argument("Simulations must be less than " +
            to_string(MAX_SIMULATIONS) + "!");

    Node* root = new Node(*root_state);
    // Build Tree
    for (uint64_t i = 0; i < simulations; i++) {
        Node* node = root->policy();
        node->rollout();
    }

    MCTS_master(root, root_state);
    deleteTreeBackground(root);
}

void MCTS_move(State *root_state, milliseconds time) {
    Node* root = new Node(*root_state);

    // How many simulations to run before checking time
    const int32_t batchSize = 1000;

    // Build Tree
    uint32_t i;
    uint32_t iterations = 0;

    auto start = high_resolution_clock::now();
    while (high_resolution_clock::now() - start < time) {
        for (i = 0; i < batchSize; i++) {
            Node* node = root->policy();
            node->rollout();
        }

        iterations++;

        if ((iterations + 1) * batchSize > MAX_SIMULATIONS) {
            printf("Exiting due to MAX_SIMULATIONS! -> Increase in Config.h\n");
            break;
        }
    }

    MCTS_master(root, root_state);
    deleteTreeBackground(root);
}

int main() {
    init();
    State state = State();
    cout << state.toString();

    const seconds aiTime = seconds(15);

    while (!state.terminal()) {
        if (!(state.getEmpty() % 2))
            MCTS_move(&state, aiTime);
        else
            human_move(&state);
        cout << state.toString();
    }
}
