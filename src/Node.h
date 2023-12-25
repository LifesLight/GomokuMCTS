#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include <vector>

#include "State.h"
#include "Statistics.h"
#include "Config.h"

using std::vector;
using std::uniform_int_distribution;
using std::mt19937;

class Node {
 public:
    Node* parent;
    uint16_t parent_action;
    Statistics* data;
    vector<Node*> children;
    vector<uint16_t> untried_actions;

    Node();
    explicit Node(State);
    explicit Node(State, Node*, uint16_t);
    explicit Node(Statistics*, Node*, uint16_t);
    explicit Node(Node*);
    ~Node();

    void rollout();
    Node* bestChild();
    // Best child without exploration biases
    Node* absBestChild();
    // Best child within confidence bound
    Node* absBestChild(float);
    Node* policy();
    int32_t qDelta(bool);

 private:
    Node* expand();
    void backpropagate(uint8_t);

    static std::random_device rd;
};
