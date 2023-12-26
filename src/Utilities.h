#pragma once

/**
 * Copyright (c) Alexander Kurtz 2023
 */

#include <stdint.h>
#include <vector>
#include <string>
#include <sstream>

#include "Config.h"

using std::vector;
using std::string;
using std::stringstream;
using std::endl;
using std::to_string;


class Utils {
 public:
    template <typename T>
    static void indexToCords(const uint16_t index, T *x, T *y) {
        *x = index % BOARD_SIZE;
        *y = index / BOARD_SIZE;
    }


    template <typename T>
    static void cordsToIndex(uint16_t *index, const T x, const T y) {
        (*index) = y * BOARD_SIZE + x;
    }

    static string cellsToString(const vector<vector<string>>& cellValues) {
        // Constants for style render style
        #ifdef _WIN32
        const string corner0 = "+";
        const string corner1 = "+";
        const string corner2 = "+";
        const string corner3 = "+";
        const string line0 = "-";
        const string line1 = "|";
        const string center = "+";
        const string cross0 = "+";
        const string cross1 = "+";
        const string cross2 = "+";
        const string cross3 = "+";
        #else
        const string corner0 = "┌";
        const string corner1 = "┐";
        const string corner2 = "└";
        const string corner3 = "┘";
        const string line0 = "─";
        const string line1 = "│";
        const string center = "┼";
        const string cross0 = "┬";
        const string cross1 = "├";
        const string cross2 = "┤";
        const string cross3 = "┴";
        #endif

        // Top line
        stringstream output;
        string three_lines = "";
        for (int i = 0; i < 3; i++)
            three_lines += line0;

        output << std::endl;
        output << "   " << corner0;
        for (int i = 0; i < BOARD_SIZE - 1; i++) {
            output << three_lines << cross0;
        }
        output << three_lines << corner1;
        output << endl;

        // Inner lines
        for (int y = BOARD_SIZE - 1; y >= 0; y--) {
            // Data line
            output << to_string(y);
            output << string(3 - to_string(y).length(), ' ');

            for (int x = 0; x < BOARD_SIZE; x++) {
                output << line1;
                output << cellValues[x][y];
            }
            output << line1;
            output << endl;

            // Row line
            if (y == 0)
                continue;

            output << "   " << cross1;
            for (int i = 0; i < BOARD_SIZE - 1; i++) {
                output << three_lines << center;
            }
            output << three_lines << cross2;
            output << endl;
        }

        // Bottom line
        output << "   " << corner2;
        for (int i = 0; i < BOARD_SIZE - 1; i++) {
            output << three_lines << cross3;
        }
        output << three_lines << corner3;
        output << endl;

        output << "    ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            output << " ";
            output << to_string(i);
            output << string(3 - to_string(i).length(), ' ');
        }

        output << endl;
        return output.str();
    }
};
