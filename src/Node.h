#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include <stdint.h>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <stack>
#include <set>

#include "Config.h"
#include "State.h"
#include "Statistics.h"
#include "Randomizer.h"

using std::unordered_map;
using std::vector;
using std::uniform_int_distribution;
using std::random_device;
using std::mt19937;
using std::begin;
using std::end;
using std::shuffle;
using std::sqrt;
using std::log;
using std::stack;
using std::endl;
using std::cout;
using std::set;


class Node {
 public:
    /**
     * Default constructor
    */
    Node();

    /**
     * Fixed state constructor
    */
    explicit Node(State state);

    /**
     * Parent data constructor
    */
    explicit Node(State state, Node* parent);

    /**
     * Inherited statistics constructor
    */
    explicit Node(Statistics* statistics, Node* parent);

    /**
     * Deconstructor
     * Recursively delete all children
    */
    ~Node();

    /**
     * Rollout from this node
     * Randomly choose actions until terminal state is reached
     * Backpropagate the result
    */
    void rollout();

    /**
     * Get the best child node for the UCT algorithm
    */
    Node* bestChild();

    /**
     * This version gets the final best child
    */
    Node* absBestChild();

    /**
    * MCTS policy algorithm
    */
    Node* policy();

    /**
     * Get nodes QDelta
     * QDelta is the difference between the results of the two players
     * Calculated from the perspective of the provided player
    */
    int32_t qDelta(bool turn);

    /**
     * Get evaluation of node
    */
    int32_t getEvaluation(const bool turn);

    /**
    * Initialize the log table
    */
    static void initLogTable();

    /**
     * Delete the transposition table
    */
    static void resetTranspositionTable();

    /**
     * Get TT hits
    */
    static double getTableHitrate();

    /**
     * Reset TT hits to 0
    */
    static void resetTTHits();

    /**
     * Iteratively delete tree
    */
    static void deleteTree(Node* root);

    /**
     * Get the state
    */
    State* getState();

    /**
     * Get Parent
    */
    Node* getParent();

    /**
     * Get Parent Action
    */
    index_t getParentAction();

    /**
     * Get children
    */
    vector<Node*>& getChildren();

    /**
     * Get untried actions
    */
    vector<index_t>& getUntriedActions();

    /**
     * Get the number of visits
    */
    uint32_t getVisits();

    /**
     * Reserve Transposition Table
    */
    static void reserveTT(uint32_t size);

    /**
     * Get empty field count
    */
    index_t getEmpty();

    /**
     * Access the scores
    */
    uint32_t getScore(uint8_t index);

    /**
     * Get child with according move
    */
    Node* getChild(index_t action);

    /**
     * Resets the node statics for a new search
    */
    static void reset();

    #ifdef RAVE
    /**
     * Get from RAVE table
    */
    static uint32_t getRaveActionVisits(index_t action);
    static void incrementRaveActionVisits(index_t action);
    static uint32_t getRaveActionResults(index_t action, uint8_t index);
    static void incrementRaveActionResults(index_t action, uint8_t index);
    static int32_t getRaveDelta(uint32_t action, bool turn);
    static void printRaveTable(bool turn);
    /**
     * Reset RAVE table
    */
    static void resetRave();
    #endif

 private:
    /**
     * Expand the node by adding a new child node.
    */
    Node* expand();

    /**
     * Backpropagate the result of a rollout
    */
    void backpropagate(uint8_t value);

    Node* parent;
    Statistics* data;
    vector<Node*> children;
    vector<index_t> untriedActions;

    /**
     * Transposition table
    */
    static unordered_map<uint64_t, Statistics*> TT;

    /**
     * TT hits
    */
    static uint32_t transposeHits;
    static uint32_t transposeMisses;

    #ifdef RAVE
    /**
     * RAVE table
    */
    static uint32_t raveVisits[BOARD_SIZE * BOARD_SIZE];
    static uint32_t raveResults[BOARD_SIZE * BOARD_SIZE][3];
    #endif

    /**
     * Log lookup table
    */
    static double logTable[MAX_SIMULATIONS];
};
