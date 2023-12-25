/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include "Node.h"

Node::Node(Statistics* data, Node* parent, uint16_t parent_action)
    : parent(parent), parent_action(parent_action), data(data) {
    untried_actions = data->state.possible();
    std::shuffle(std::begin(untried_actions), std::end(untried_actions), rng);
}

Node::Node(State state, Node* parent, uint16_t parent_action)
    : Node(new Statistics(state), parent, parent_action) {  }

Node::Node(State state)
    : Node(state, nullptr, 0) {  }

Node::Node()
    : Node(new State()) {  }

Node::~Node() {
    for (Node* child : children) delete child;
}

Node* Node::expand() {
        uint16_t index = untried_actions.back();
        untried_actions.pop_back();

        State resulting_state(data->state);
        resulting_state.action(index);

        Node* child;
        Statistics* child_stats;
        auto TT_stats = TT->find(resulting_state.hash_value);

        if (TT_stats != TT->end()) {
            TT_hits++;
            child_stats = TT_stats->second;
            child = new Node(child_stats, this, index);
            children.push_back(child);
            return this->policy();
        }

        child_stats = new Statistics(resulting_state);
        child = new Node(child_stats, this, index);
        TT->insert({ resulting_state.hash_value, child_stats });
        children.push_back(child);

        return child;
}

void Node::rollout() {
    const i16_t upperBound = untried_actions.size();

    State simulation_state = State(data->state);

    uniform_int_distribution<mt19937::result_type> distribution(0, upperBound);
    uint16_t index = distribution(rng);
    while (!simulation_state.terminal()) {
        simulation_state.action(untried_actions[index % upperBound]);
        index++;
    }
    backpropagate(simulation_state.result);
}

void Node::backpropagate(uint8_t value) {
    data->visits++;
    data->results[value]++;
    if (parent)
        parent->backpropagate(value);
}

int32_t Node::qDelta(bool turn) {
    if (turn)   return data->results[0] - data->results[1];
    else        return data->results[1] - data->results[0];
}

Node* Node::bestChild() {
    Node* best_child = nullptr;
    FloatPrecision best_result = -100.0;
    // Precompute
    FloatPrecision log_visits = 2 * logTable[data->visits];
    bool turn = data->state.empty % 2;
    FloatPrecision Q_value;
    FloatPrecision result;
    for (Node* child : children) {
        Q_value = FloatPrecision(child->qDelta(turn)) / FloatPrecision(child->data->visits);
        result = Q_value + ExplorationBias * std::sqrt(log_visits / FloatPrecision(child->data->visits));
        if (result > best_result)
        {
            best_result = result;
            best_child = child;
        }
    }
    return best_child;
}

Node* Node::absBestChild()
{
    Node* best_child = nullptr;
    FloatPrecision result;
    FloatPrecision best_result = -100.0;
    // Precompute
    bool turn = data->state.empty % 2;
    for (Node* child : children)
    {
        FloatPrecision Q_value = FloatPrecision(child->qDelta(turn)) / FloatPrecision(child->data->visits);
        result = Q_value;
        if (result > best_result)
        {
            best_result = result;
            best_child = child;
        }
    }
    return best_child;
}

Node* Node::absBestChild(float confidence_bound)
{
    std::list<Node*> children_copy;
    for (Node* child : children)
       if (FloatPrecision(child->data->visits) / FloatPrecision(data->visits) > confidence_bound)
            children_copy.push_back(child);
 
    Node* best_child = nullptr;
    FloatPrecision result;
    FloatPrecision best_result = -100.0;
    // Precompute
    bool turn = data->state.empty % 2;
    
    for (Node* child : children_copy)
    {
        FloatPrecision Q_value = FloatPrecision(child->qDelta(turn)) / FloatPrecision(child->data->visits);
        result = Q_value;
        if (result > best_result)
        {
            best_result = result;
            best_child = child;
        }
    }

    return best_child;
}

Node* Node::policy()
{
    Node* current = this;
    while (!current->data->state.terminal())
        if (current->untried_actions.size() > 0)
            return current->expand();
        else
            current = current->bestChild();
    return current;
}
