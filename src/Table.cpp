/**
 * Copyright (c) Alexander Kurtz 2023
 */

#include "Table.h"

Table::Table() { }

Table::~Table() { }

size_t Table::index(const State& state) const {
    return state.getHash() % size;
}

void Table::put(const State& state, Statistics* stats) {
    const size_t hash = index(state);
    if (table[hash] != nullptr) {
        printf("WARNING: Hash collision detected!\n");
    }
    table[hash] = stats;
}

Statistics* Table::get(const State& state) const {
    const size_t hash = index(state);
    return table[hash];
}

void Table::setSize(int32_t size) {
    this->size = size;
    table.resize(size);
    clear();
}

uint32_t Table::getSize() const {
    return size;
}

void Table::clear() {
    std::fill(table.begin(), table.end(), nullptr);
}
