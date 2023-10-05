#pragma once

#include "Definitions.h"
#include "State.h"
#include "Statistics.h"
#include "Config.h"

class Node
{
public:
    Node* parent;
    uint16_t parent_action;
    Statistics* data;
    std::list<Node*> children;
    std::vector<uint16_t> untried_actions;

    Node();
    Node(State);
    Node(State, Node*, uint16_t);
    Node(Statistics*, Node*, uint16_t);
    Node(Node*);
    ~Node();

    void rollout();
    Node* bestChild();
    Node* policy();
    int32_t qDelta(bool);

private:
    Node* expand();
    void backpropagate(uint8_t);
};
