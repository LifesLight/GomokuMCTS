/**
 * Copyright (c) Alexander Kurtz 2023
 */

#include "Dev.h"

void Analytics::visitsDist(Node* root) {
    vector<vector<string>> cellValues;

    uint32_t maxVisits = 0;
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
        Node* child = root->getChild(i);
        if (child != nullptr) {
            if (child->getVisits() > maxVisits)
                maxVisits = child->getVisits();
        }
    }

    for (int i = 0; i < BOARD_SIZE; i++) {
        vector<string> row;
        for (int j = 0; j < BOARD_SIZE; j++) {
            index_t index;
            Utils::cordsToIndex(&index, i, j);
            Node* child = root->getChild(index);
            if (child != nullptr) {
                // Add relative visits in percent with 3 digits padding
                // The value is a integer from 0 to 999
                const int visits = (child->getVisits() * 999) / maxVisits;
                ostringstream ss;
                ss << setw(3) << setfill(' ') << visits;
                row.push_back(ss.str());
            } else {
                row.push_back(" x ");
            }
        }
        cellValues.push_back(row);
    }

    cout << endl << "    < Visits Distribution >" << endl;
    cout << Utils::cellsToString(cellValues);
}

void Analytics::ucbDist(Node* root) {
    vector<vector<string>> cellValues;

    const bool turn = root->getState()->getEmpty() % 2;

    for (int i = 0; i < BOARD_SIZE; i++) {
        vector<string> row;
        for (int j = 0; j < BOARD_SIZE; j++) {
            index_t index;
            Utils::cordsToIndex(&index, i, j);
            Node* child = root->getChild(index);
            if (child != nullptr) {
                // Add relative visits in percent with 3 digits padding
                // The value is a integer from 0 to 999
                const int visits = child->qDelta(turn) /
                    static_cast<double>(child->getVisits()) * 99;
                ostringstream ss;
                ss << std::setw(3) << std::setfill(' ') << visits;
                row.push_back(ss.str());
            } else {
                row.push_back(" x ");
            }
        }
        cellValues.push_back(row);
    }

    cout << endl << "    < UCB Distribution >" << endl;
    cout << Utils::cellsToString(cellValues);
}

#ifdef RAVE
void Analytics::raveDist(Node* root) {
    vector<vector<string>> cellValues;

    const bool turn = root->getState()->getEmpty() % 2;

    for (int i = 0; i < BOARD_SIZE; i++) {
        vector<string> row;
        for (int j = 0; j < BOARD_SIZE; j++) {
            index_t index;
            Utils::cordsToIndex(&index, i, j);
            Node* child = root->getChild(index);
            if (child != nullptr) {
                // Add relative visits in percent with 3 digits padding
                // The value is a integer from 0 to 999
                const int visits = child->getRaveDelta(index, turn) /
                    static_cast<double>(child->getRaveActionVisits(index)) * 99;
                ostringstream ss;
                ss << std::setw(3) << std::setfill(' ') << visits;
                row.push_back(ss.str());
            } else {
                row.push_back(" x ");
            }
        }
        cellValues.push_back(row);
    }

    cout << endl << "    < RAVE Distribution >" << endl;
    cout << Utils::cellsToString(cellValues);
}
#endif

void Analytics::overview(Node* best) {
    ostringstream result;

    result << "    <";
    for (index_t i = 0; i < BOARD_SIZE * 2 + 26; i++)
        result << "-";
    result << ">\n";

    int x, y;
    Utils::indexToCords(best->getParentAction(), &x, &y);

    Node* parent = best->getParent();
    const double simsInMillion = (parent->getVisits() / 1000) / 1000.0;
    const int tRelWins = parent->getScore(best->getEmpty() % 2);
    const int tRelLoss = parent->getScore(!best->getEmpty() % 2);
    const int tRelDraw = parent->getScore(2);
    const int fRelWins = best->getScore(best->getEmpty() % 2);
    const int fRelLoss = best->getScore(!best->getEmpty() % 2);
    const int fRelDraw = best->getScore(2);
    const double evaluation = best->qDelta(parent->getEmpty() % 2) /
        static_cast<double>(best->getVisits());
    const double TT_hitrate = Node::getTableHitrate();
    const double confidence = (best->getVisits() * 100) /
        static_cast<double>(parent->getVisits());
    const double draw = (best->getScore(2) * 100) /
        static_cast<double>(best->getVisits());

    result << "Action:      " << x << "," << y << "\n";
    result << "Simulations: " << simsInMillion << "M"
        << " (W:" << tRelWins
        << " L:" << tRelLoss
        << " D:" << tRelDraw << ")\n";
    result << "Evaluation:  " << fixed << setprecision(3) << evaluation
       << " (W:" << fRelWins
       << " L:" << fRelLoss
       << " D:" << fRelDraw << ")\n";
    result << "TT-Hitrate:  " << TT_hitrate << "%\n";
    result << "Confidence:  " << confidence << "%\n";
    result << "Draw:        " << draw << "%\n";

    result << "    <";
    for (index_t i = 0; i < BOARD_SIZE * 2 + 26; i++)
        result << "-";
    result << ">\n";

    cout << result.str();
}
