/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include "Node.h"

unordered_map<uint64_t, Statistics*> Node::TT;

double Node::logTable[MAX_SIMULATIONS];

uint32_t Node::TransposeHits = 0;

Node::Node(Statistics* data, Node* parent)
    : parent(parent), data(data) {

    // Shuffle actions for faster rollout
    untriedActions = data->state.possible();
    shuffle(begin(untriedActions), end(untriedActions), Randomizer::getRng());
}

Node::Node(State state, Node* parent)
    : Node(new Statistics(state), parent) {  }

Node::Node(State state)
    : Node(state, nullptr) {  }

Node::Node()
    : Node(State()) {  }

Node::~Node() { }

Node* Node::expand() {
        // Decide which action to take
        const index_t index = untriedActions.back();
        untriedActions.pop_back();

        // Create matching state
        State resultingState(data->state);
        resultingState.action(index);

        // Check if state is in TT
        Node* child;
        Statistics* childStats;
        auto transposeStats = Node::TT.find(resultingState.getHash());

        // If state is in TT, use its statistics
        if (transposeStats != TT.end()) {
            Node::TransposeHits++;
            childStats = transposeStats->second;
            child = new Node(childStats, this);
            children.push_back(child);
            return this->policy();
        }

        // If state is not in TT, create new statistics
        childStats = new Statistics(resultingState);
        child = new Node(childStats, this);
        TT.insert({ resultingState.getHash(), childStats });
        children.push_back(child);

        return child;
}

void Node::rollout() {
    // Max amount of actions
    const int16_t maxActions = untriedActions.size();

    State simulationState = State(data->state);

    index_t index = Randomizer::randomInt<index_t>(maxActions);

    // Make random actions until terminal state is reached
    while (!simulationState.terminal()) {
        // Loop around
        // This is faster than modulo
        if (index == maxActions) {
            index = 0;
        }

        simulationState.action(untriedActions[index]);
        index++;
    }

    // Backpropagate result
    backpropagate(simulationState.getResult());
}

void Node::backpropagate(const uint8_t value) {
    // Update statistics
    data->visits++;
    data->results[value]++;
    // If parent exists, backpropagate
    if (parent)
        parent->backpropagate(value);
}

int32_t Node::qDelta(const bool turn) {
    if (turn)   return data->results[0] - data->results[1];
    else        return data->results[1] - data->results[0];
}

Node* Node::bestChild() {
    Node* bestChild = nullptr;

    // Best result of UCT
    double bestResult = -100.0;

    // Precompute
    const double logVisits = 2 * Node::logTable[data->visits];

    // Needed for remaining code
    const bool turn = data->state.getEmpty() % 2;
    double evaluation, result;

    // Iterate over all children
    for (Node* child : children) {
        // Node value
        evaluation = child->getEvaluation(turn) /
            static_cast<double>(child->data->visits);

        // Account for exploration bias
        result = evaluation +
            EXPLORATION_BIAS *
            sqrt(logVisits / static_cast<double>(child->data->visits));

        // Update best child
        if (result > bestResult) {
            bestResult = result;
            bestChild = child;
        }
    }

    return bestChild;
}

Node* Node::absBestChild() {
    // Needed for remaining code
    Node* bestChild = nullptr;
    int32_t result;
    int32_t bestResult = -100.0;

    // Precompute
    const bool turn = data->state.getEmpty() % 2;

    // Find child with most visits
    for (Node* child : children) {
        result = child->getVisits();

        if (result > bestResult) {
            bestResult = result;
            bestChild = child;
        }
    }

    return bestChild;
}

Node* Node::policy() {
    Node* current = this;
    while (!current->data->state.terminal())
        if (current->untriedActions.size() > 0)
            return current->expand();
        else
            current = current->bestChild();
    return current;
}


void Node::initLogTable() {
    for (int i = 0; i < MAX_SIMULATIONS; i++)
        Node::logTable[i] = log(i);
}

uint32_t Node::getTTHits() {
    return Node::TransposeHits;
}

void Node::resetTTHits() {
    Node::TransposeHits = 0;
}

void Node::resetTranspositionTable() {
    Node::TT.clear();
    Node::resetTTHits();
}

void Node::deleteTree(Node* root) {
    stack<Node*> nodes;
    nodes.push(root);

    // Keep track of deleted statistics
    set<Statistics*> deletedData;

    while (!nodes.empty()) {
        Node* current = nodes.top();
        nodes.pop();

        for (Node* child : current->children) {
            nodes.push(child);
        }

        // Delete data if not already deleted
        if (deletedData.find(current->data) == deletedData.end()) {
            delete current->data;
            deletedData.insert(current->data);
        }

        delete current;
    }
}

int32_t Node::getEvaluation(const bool turn) {
    int32_t eval = qDelta(turn);
    return eval;
}

State* Node::getState() {
    return &data->state;
}

Node* Node::getParent() {
    return parent;
}

index_t Node::getParentAction() {
    return data->state.getLast();
}

vector<Node*>& Node::getChildren() {
    return children;
}

vector<index_t>& Node::getUntriedActions() {
    return untriedActions;
}

uint32_t Node::getVisits() {
    return data->visits;
}

void Node::reserveTT(uint32_t size) {
    Node::TT.reserve(size);
}

uint32_t Node::getScore(uint8_t index) {
    return data->results[index];
}

index_t Node::getEmpty() {
    return data->state.getEmpty();
}
