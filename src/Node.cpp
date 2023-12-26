/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include "Node.h"

unordered_map<uint64_t, Statistics*> Node::TT;

double Node::log_table[MAX_SIMULATIONS];

uint32_t Node::TT_hits = 0;

Node::Node(Statistics* data, Node* parent)
    : parent(parent), data(data) {

    // Shuffle actions for faster rollout
    untried_actions = data->state.possible();
    shuffle(begin(untried_actions), end(untried_actions), Randomizer::getRng());
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
        const index_t index = untried_actions.back();
        untried_actions.pop_back();

        // Create matching state
        State resulting_state(data->state);
        resulting_state.action(index);

        // Check if state is in TT
        Node* child;
        Statistics* child_stats;
        auto TT_stats = Node::TT.find(resulting_state.get_hash());

        // If state is in TT, use its statistics
        if (TT_stats != TT.end()) {
            Node::TT_hits++;
            child_stats = TT_stats->second;
            child = new Node(child_stats, this);
            children.push_back(child);
            return this->policy();
        }

        // If state is not in TT, create new statistics
        child_stats = new Statistics(resulting_state);
        child = new Node(child_stats, this);
        TT.insert({ resulting_state.get_hash(), child_stats });
        children.push_back(child);

        return child;
}

void Node::rollout() {
    // Max amount of actions
    const int16_t max_actions = untried_actions.size();

    State simulation_state = State(data->state);

    index_t index = Randomizer::randomInt<index_t>(max_actions);

    // Make random actions until terminal state is reached
    while (!simulation_state.terminal()) {
        // Loop around
        // This is faster than modulo
        if (index == max_actions) {
            index = 0;
        }

        simulation_state.action(untried_actions[index]);
        index++;
    }

    // Backpropagate result
    backpropagate(simulation_state.get_result());
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
    Node* best_child = nullptr;

    // Best result of UCT
    double best_result = -100.0;

    // Precompute
    const double log_visits = 2 * Node::log_table[data->visits];

    // Needed for remaining code
    const bool turn = data->state.get_empty() % 2;
    double Q_value, result;

    // Iterate over all children
    for (Node* child : children) {
        // Node value
        Q_value = child->qDelta(turn) /
            static_cast<double>(child->data->visits);

        // Account for exploration bias
        result = Q_value +
            EXPLORATION_BIAS *
            sqrt(log_visits / static_cast<double>(child->data->visits));

        // Update best child
        if (result > best_result) {
            best_result = result;
            best_child = child;
        }
    }

    return best_child;
}

Node* Node::absBestChild() {
    // Needed for remaining code
    Node* best_child = nullptr;
    int32_t result;
    int32_t best_result = -100.0;

    // Precompute
    const bool turn = data->state.get_empty() % 2;

    // Find child with most visits
    for (Node* child : children) {
        result = child->data->visits;

        if (result > best_result) {
            best_result = result;
            best_child = child;
        }
    }

    return best_child;
}

Node* Node::policy() {
    Node* current = this;
    while (!current->data->state.terminal())
        if (current->untried_actions.size() > 0)
            return current->expand();
        else
            current = current->bestChild();
    return current;
}


void Node::initLogTable() {
    for (int i = 0; i < MAX_SIMULATIONS; i++)
        Node::log_table[i] = log(i);
}

uint32_t Node::getTTHits() {
    return Node::TT_hits;
}

void Node::resetTTHits() {
    Node::TT_hits = 0;
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

State* Node::getState() {
    return &data->state;
}

Node* Node::getParent() {
    return parent;
}

index_t Node::getParentAction() {
    return data->state.get_last();
}

vector<Node*>& Node::getChildren() {
    return children;
}

vector<index_t>& Node::getUntriedActions() {
    return untried_actions;
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
    return data->state.get_empty();
}
