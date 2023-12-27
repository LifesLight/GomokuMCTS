/**
 * Copyright (c) Alexander Kurtz 2023
 */


#include "State.h"

/**
 * Initialize Zobrist Hashing Table
*/
vector<vector<int64_t>> State::zobristTable(
    BOARD_SIZE * BOARD_SIZE, vector<int64_t>(3));

State::State()
    : last(0), empty(BOARD_SIZE * BOARD_SIZE), result(2) {
    memset(mArray, 0, sizeof(block_t) * BOARD_SIZE * 6);
    memset(cArray, 0, sizeof(block_t) * BOARD_SIZE * 6);
    hashValue = hash();
}

State::State(State* source)
    :   last(source->last), empty(source->empty), result(source->result),
        hashValue(source->hashValue) {
    memcpy(mArray, source->mArray, sizeof(block_t) * BOARD_SIZE * 6);
    memcpy(cArray, source->cArray, sizeof(block_t) * BOARD_SIZE * 6);
}

void State::action(const uint8_t x, const uint8_t y) {
    index_t index;
    Utils::cordsToIndex(&index, x, y);
    action(index);
}

void State::action(const index_t index) {
    --empty;
    last = index;
    block_t x, y;
    Utils::indexToCords(index, &x, &y);

    // Horizontal
    mArray[y] |= (block_t(1) << x);
    // Vertical
    mArray[x + BOARD_SIZE] |= (block_t(1) << y);
    // LDiagonal
    mArray[x + BOARD_SIZE - 1 - y + BOARD_SIZE * 2] |= (block_t(1) << x);
    // RDiagonal
    mArray[BOARD_SIZE - 1 - x + BOARD_SIZE - 1 - y + BOARD_SIZE * 4] |=
        (block_t(1) << x);
    // Flip Colors
    for (index_t i = 0; i < BOARD_SIZE * 6; i++) cArray[i] ^= mArray[i];

    // Update hash
    hashValue ^= zobristTable[index][0];
    if (empty % 2)
        hashValue ^= zobristTable[index][1];
    else
        hashValue ^= zobristTable[index][2];

    // Check for 5-Stone alignment
    result = checkForFive() ? empty % 2 : 2;
}

std::vector<index_t> State::possible() const {
    // Vector of possible actions
    std::vector<index_t> actions;

    // Index of possible action
    index_t index;

    // Reserve enough space
    actions.reserve(empty);

    // Find empty fields
    for (index_t x = 0; x < BOARD_SIZE; x++) {
        for (index_t y = 0; y < BOARD_SIZE; y++) {
            if (isEmpty(x, y)) {
                Utils::cordsToIndex(&index, x, y);
                actions.push_back(index);
            }
        }
    }

    return actions;
}

bool State::terminal() const {
    return (empty == 0 || result < 2);
}

int8_t State::getCellValue(index_t index) const {
    uint8_t x, y;
    Utils::indexToCords(index, &x, &y);
    return getCellValue(x, y);
}

int8_t State::getCellValue(uint8_t x, uint8_t y) const {
    if (mArray[y] & (block_t(1) << x)) {
        if (cArray[y] & (block_t(1) << x))
            return empty % 2;
        return !(empty % 2);
    }

    return -1;
}

string State::toString() const {
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

bool State::checkForFive() {
    uint8_t x = last % BOARD_SIZE;
    uint8_t y = last / BOARD_SIZE;
#if BOARD_SIZE < 16
    uint64_t m = ((uint64_t)cArray[y] << 48)
        + ((uint64_t)cArray[x + BOARD_SIZE] << 32)
        + ((uint64_t)cArray[x + BOARD_SIZE - 1 - y + BOARD_SIZE * 2] << 16)
        + ((uint64_t)cArray[
            BOARD_SIZE - 1 - x + BOARD_SIZE - 1 - y + BOARD_SIZE * 4]);

    m &= (m >> uint64_t(1));
    m &= (m >> uint64_t(2));
    return (m & (m >> uint64_t(1)));
#else
    // Horizontal
    block_t m = cArray[y];
    m = m & (m >> block_t(1));
    m = (m & (m >> block_t(2)));
    if (m & (m >> block_t(1))) return true;
    // Vertical
    m = cArray[x + BOARD_SIZE];
    m = m & (m >> block_t(1));
    m = (m & (m >> block_t(2)));
    if (m & (m >> block_t(1))) return true;
    // LDiagonal
    m = cArray[x + BOARD_SIZE - 1 - y + BOARD_SIZE * 2];
    m = m & (m >> block_t(1));
    m = (m & (m >> block_t(2)));
    if (m & (m >> block_t(1))) return true;
    // RDiagonal
    m = cArray[BOARD_SIZE - 1 - x + BOARD_SIZE - 1 - y + BOARD_SIZE * 4];
    m = m & (m >> block_t(1));
    m = (m & (m >> block_t(2)));
    if (m & (m >> block_t(1))) return true;
    return false;
#endif
}

// Gets value for empty field, updates progressively
uint64_t State::hash() {
    uint64_t hashValue = 0;
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i)
        hashValue ^= State::zobristTable[i][0];
    return hashValue;
}

void State::initZobrist() {
    // Init Zobrist Hashing Table
    std::uniform_int_distribution<int64_t> distribution;
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i)
        for (int j = 0; j < 3; ++j)
            zobristTable[i][j] = distribution(Randomizer::getRng());
}

bool State::isEmpty(const index_t index) const {
    uint8_t x, y;
    Utils::indexToCords(index, &x, &y);
    return isEmpty(x, y);
}

bool State::isEmpty(const uint8_t x, const uint8_t y) const {
    return !(mArray[y] & (block_t(1) << x));
}

uint8_t State::getResult() const {
    return result;
}

index_t State::getLast() const {
    return last;
}

index_t State::getEmpty() const {
    return empty;
}

uint64_t State::getHash() const {
    return hashValue;
}
