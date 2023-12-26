/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include "State.h"

/**
 * Initialize Zobrist Hashing Table
*/
vector<vector<int64_t>> State::zobrist_table(
    BOARD_SIZE * BOARD_SIZE, vector<int64_t>(3));

State::State()
    : last(0), empty(BOARD_SIZE * BOARD_SIZE), result(2) {
    memset(m_array, 0, sizeof(block_t) * BOARD_SIZE * 6);
    memset(c_array, 0, sizeof(block_t) * BOARD_SIZE * 6);
    hash_value = hash();
}

State::State(State* source)
    :   last(source->last), empty(source->empty), result(source->result),
        hash_value(source->hash_value) {
    memcpy(m_array, source->m_array, sizeof(block_t) * BOARD_SIZE * 6);
    memcpy(c_array, source->c_array, sizeof(block_t) * BOARD_SIZE * 6);
}

void State::action(const uint8_t x, const uint8_t y) {
    uint16_t index;
    Utils::cordsToIndex(&index, x, y);
    action(index);
}

void State::action(const uint16_t index) {
    --empty;
    last = index;
    block_t x, y;
    Utils::indexToCords(index, &x, &y);

    // Horizontal
    m_array[y] |= (block_t(1) << x);
    // Vertical
    m_array[x + BOARD_SIZE] |= (block_t(1) << y);
    // LDiagonal
    m_array[x + BOARD_SIZE - 1 - y + BOARD_SIZE * 2] |= (block_t(1) << x);
    // RDiagonal
    m_array[BOARD_SIZE - 1 - x + BOARD_SIZE - 1 - y + BOARD_SIZE * 4] |=
        (block_t(1) << x);
    // Flip Colors
    for (uint16_t i = 0; i < BOARD_SIZE * 6; i++) c_array[i] ^= m_array[i];

    // Update hash
    hash_value ^= zobrist_table[index][0];
    if (empty % 2)
        hash_value ^= zobrist_table[index][1];
    else
        hash_value ^= zobrist_table[index][2];

    // Check for 5-Stone alignment
    result = check_for_5() ? empty % 2 : 2;
}

std::vector<uint16_t> State::possible() {
    // Vector of possible actions
    std::vector<uint16_t> actions;

    // Index of possible action
    uint16_t index;

    // Reserve enough space
    actions.reserve(empty);

    // Find empty fields
    for (uint16_t x = 0; x < BOARD_SIZE; x++) {
        for (uint16_t y = 0; y < BOARD_SIZE; y++) {
            if (is_empty(x, y)) {
                Utils::cordsToIndex(&index, x, y);
                actions.push_back(index);
            }
        }
    }

    return actions;
}

bool State::terminal() {
    return (empty == 0 || result < 2);
}

int8_t State::getCellValue(uint16_t index) {
    uint8_t x, y;
    Utils::indexToCords(index, &x, &y);
    return getCellValue(x, y);
}

int8_t State::getCellValue(uint8_t x, uint8_t y) {
    if (m_array[y] & (block_t(1) << x)) {
        if (c_array[y] & (block_t(1) << x))
            return empty % 2;
        return !(empty % 2);
    }

    return -1;
}

string State::toString() {
    // Constants for rendering
    const string stoneBlack = " ● ";
    const string stoneWhite = " ● ";
    const string colorBlack = "\033[0;34m";
    const string colorWhite = "\033[0;31m";
    const string resetColor = "\033[0m";

    vector<vector<string>> cellValues;
    for (int x = 0; x < BOARD_SIZE; x++) {
        vector<string> column;
        for (int y = 0; y < BOARD_SIZE; y++) {
            string value;
            int8_t index_value = getCellValue(x, y);
            if (index_value == -1) {
                value += "   ";
            } else if (index_value == 0) {
                value += colorBlack;
                value += stoneBlack;
                value += resetColor;
            } else {
                value += colorWhite;
                value += stoneWhite;
                value += resetColor;
            }
            column.push_back(value);
        }
        cellValues.push_back(column);
    }

    return Utils::cellsToString(cellValues);
}

bool State::check_for_5() {
    uint8_t x = last % BOARD_SIZE;
    uint8_t y = last / BOARD_SIZE;
#if BOARD_SIZE < 16
    uint64_t m = ((uint64_t)c_array[y] << 48)
        + ((uint64_t)c_array[x + BOARD_SIZE] << 32)
        + ((uint64_t)c_array[x + BOARD_SIZE - 1 - y + BOARD_SIZE * 2] << 16)
        + ((uint64_t)c_array[
            BOARD_SIZE - 1 - x + BOARD_SIZE - 1 - y + BOARD_SIZE * 4]);

    m &= (m >> uint64_t(1));
    m &= (m >> uint64_t(2));
    return (m & (m >> uint64_t(1)));
#else
    // Horizontal
    block_t m = c_array[y];
    m = m & (m >> block_t(1));
    m = (m & (m >> block_t(2)));
    if (m & (m >> block_t(1))) return true;
    // Vertical
    m = c_array[x + BOARD_SIZE];
    m = m & (m >> block_t(1));
    m = (m & (m >> block_t(2)));
    if (m & (m >> block_t(1))) return true;
    // LDiagonal
    m = c_array[x + BOARD_SIZE - 1 - y + BOARD_SIZE * 2];
    m = m & (m >> block_t(1));
    m = (m & (m >> block_t(2)));
    if (m & (m >> block_t(1))) return true;
    // RDiagonal
    m = c_array[BOARD_SIZE - 1 - x + BOARD_SIZE - 1 - y + BOARD_SIZE * 4];
    m = m & (m >> block_t(1));
    m = (m & (m >> block_t(2)));
    if (m & (m >> block_t(1))) return true;
    return false;
#endif
}

// Gets value for empty field, updates progressively
uint64_t State::hash() {
    uint64_t hash_value = 0;
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i)
        hash_value ^= State::zobrist_table[i][0];
    return hash_value;
}

void State::init_zobrist() {
    // Init Zobrist Hashing Table
    std::uniform_int_distribution<int64_t> distribution;
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i)
        for (int j = 0; j < 3; ++j)
            zobrist_table[i][j] = distribution(Randomizer::getRng());
}

bool State::is_empty(const uint16_t index) {
    uint8_t x, y;
    Utils::indexToCords(index, &x, &y);
    return is_empty(x, y);
}

bool State::is_empty(const uint8_t x, const uint8_t y) {
    return !(m_array[y] & (block_t(1) << x));
}

uint8_t State::get_result() {
    return result;
}

uint16_t State::get_last() {
    return last;
}

uint16_t State::get_empty() {
    return empty;
}

uint64_t State::get_hash() {
    return hash_value;
}
