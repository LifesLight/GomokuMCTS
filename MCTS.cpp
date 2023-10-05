#pragma region 
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <string>
#include <chrono>
#include <random>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <cstring>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#pragma endregion

//#define RAVE

#define SIZE 15
#define BIAS 1.4142
#define RAVE_BIAS 0.4
#define BATCH_SIZE 1000
#define MAX_SIMULATIONS 250000000
typedef float PREC;

#pragma region 
#if SIZE > 32
typedef int64_t BLOCK;
#elif SIZE > 16
typedef int32_t BLOCK;
#elif SIZE > 8
typedef int16_t BLOCK;
#else
typedef int8_t BLOCK;
#endif
#pragma endregion

std::random_device rand_device;
std::mt19937 rng(rand_device());
PREC log_table[MAX_SIMULATIONS];
uint64_t log_table_size = 0;

class NODE;
class STATISTICS;

const uint64_t SEED = 0x1e1700fa712fc381;
std::vector<std::vector<int64_t>> zobrist_table(SIZE* SIZE, std::vector<int64_t>(3));
std::unordered_map<uint64_t, STATISTICS*>* TT = new std::unordered_map<uint64_t, STATISTICS*>;
uint32_t TT_subcheck = 0;
uint32_t TT_hits = 0;

class STATE
{
public:

    /* Partitions :
    -----------------------------------------------------------
    1: Horizontal   2: Vertical     3: LDiagonal    4: RDiagonal
    xxxx            xxxo            oooo            oooo
    xoxx            xoxo            xooo            oooo
    xxoo            xxoo            xxoo            xooo
    oooo            xxoo            xooo            xxxo
                                    xxoo            xoxo
                                    xxoo            xxoo
                                    xooo            xooo
    ----------------------------------------------------------- */

    BLOCK m_array[SIZE * 6];
    BLOCK c_array[SIZE * 6];
    uint16_t last, empty;
    // 0:p0win 1:p1win 2:none
    uint8_t result;
    uint64_t hash_value;

    STATE()
        : last(0), empty(SIZE* SIZE), result(2)
    {
        memset(m_array, 0, sizeof(BLOCK) * SIZE * 6);
        memset(c_array, 0, sizeof(BLOCK) * SIZE * 6);
        hash_value = hash();
    }

    STATE(STATE* source)
        : last(source->last), empty(source->empty), result(source->result), hash_value(source->hash_value)
    {
        memcpy(m_array, source->m_array, sizeof(BLOCK) * SIZE * 6);
        memcpy(c_array, source->c_array, sizeof(BLOCK) * SIZE * 6);
    }

    void action(uint16_t index)
    {
        --empty;
        last = index;
        BLOCK x = index % SIZE;
        BLOCK y = index / SIZE;
        // Horizontal
        m_array[y] |= (BLOCK(1) << x);
        // Vertical
        m_array[x + SIZE] |= (BLOCK(1) << y);
        // LDiagonal
        m_array[x + SIZE - 1 - y + SIZE * 2] |= (BLOCK(1) << x);
        // RDiagonal
        m_array[SIZE - 1 - x + SIZE - 1 - y + SIZE * 4] |= (BLOCK(1) << x);
        // Flip Colors
        for (uint16_t i = 0; i < SIZE * 6; i++) c_array[i] ^= m_array[i];

        // Update hash
        hash_value ^= zobrist_table[index][0];
        if (empty % 2)
            hash_value ^= zobrist_table[index][1];
        else
            hash_value ^= zobrist_table[index][2];

        // Check for 5-Stone alignment
        result = check_for_5() ? empty % 2 : 2;
    }

    std::vector<uint16_t> possible()
    {
        std::vector<uint16_t> actions;
        actions.reserve(empty);
        for (uint16_t i = 0; i < SIZE * SIZE; i++)
            if (!(m_array[i / SIZE] & (BLOCK(1) << i % SIZE))) actions.push_back(i);
        return actions;
    }

    bool terminal()
    {
        return (empty == 0 || result < 2);
    }

    void render()
    {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
        std::cout << "    ";
        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << " " << std::to_string(i).append(3 - std::to_string(i).length(), ' ');
        std::cout << "\n   ";
        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << " ---";
        std::cout << "\n";
        for (uint16_t y = 0; y < SIZE; y++)
        {
            std::cout << std::to_string(y).append(3 - std::to_string(y).length(), ' ');
            for (uint16_t x = 0; x < SIZE; x++)
            {
                std::cout << "|";
                if (!(m_array[y] & (BLOCK(1) << x)))
                    std::cout << "   ";
                else if (c_array[y] & (BLOCK(1) << x))
                {
#ifdef _WIN32
                    SetConsoleTextAttribute(hConsole, empty % 2 ? 9 : 4);
                    std::cout << " o ";
                    SetConsoleTextAttribute(hConsole, 7);
#else
                    if (!(empty % 2))   std::cout << "\033[1;34m o \033[0m";
                    else                std::cout << "\033[1;31m o \033[0m";
#endif
                }
                else if (!(c_array[y] & (BLOCK(1) << x)))
                {
#ifdef _WIN32
                    SetConsoleTextAttribute(hConsole, empty % 2 ? 4 : 9);
                    std::cout << " o ";
                    SetConsoleTextAttribute(hConsole, 7);
#else
                    if (empty % 2)      std::cout << "\033[1;34m o \033[0m";
                    else                std::cout << "\033[1;31m o \033[0m";
#endif
                }
            }
            std::cout << "|\n   ";
            for (uint16_t i = 0; i < SIZE; i++)
                std::cout << " ---";
            std::cout << "\n";
        }
    }

private:
    // Gets value for empty field, updates progressively 
    uint64_t hash()
    {
        uint64_t hash_value = 0;
        for (int i = 0; i < SIZE * SIZE; ++i)
            hash_value ^= zobrist_table[i][0];
        return hash_value;
    }

    bool check_for_5()
    {
        uint8_t x = last % SIZE;
        uint8_t y = last / SIZE;
#if SIZE < 16
        uint64_t m = ((uint64_t)c_array[y] << 48)
            + ((uint64_t)c_array[x + SIZE] << 32)
            + ((uint64_t)c_array[x + SIZE - 1 - y + SIZE * 2] << 16)
            + ((uint64_t)c_array[SIZE - 1 - x + SIZE - 1 - y + SIZE * 4]);

        m &= (m >> uint64_t(1));
        m &= (m >> uint64_t(2));
        return (m & (m >> uint64_t(1)));
#else
        //Horizontal
        BLOCK m = c_array[y];
        m = m & (m >> BLOCK(1));
        m = (m & (m >> BLOCK(2)));
        if (m & (m >> BLOCK(1))) return true;
        //Vertical
        m = c_array[x + SIZE];
        m = m & (m >> BLOCK(1));
        m = (m & (m >> BLOCK(2)));
        if (m & (m >> BLOCK(1))) return true;
        //LDiagonal
        m = c_array[x + SIZE - 1 - y + SIZE * 2];
        m = m & (m >> BLOCK(1));
        m = (m & (m >> BLOCK(2)));
        if (m & (m >> BLOCK(1))) return true;
        //RDiagonal
        m = c_array[SIZE - 1 - x + SIZE - 1 - y + SIZE * 4];
        m = m & (m >> BLOCK(1));
        m = (m & (m >> BLOCK(2)));
        if (m & (m >> BLOCK(1))) return true;
        return false;
#endif
    }
};

class STATISTICS
{
public:
    STATE state;
    uint32_t visits;
    uint32_t results[3];

    STATISTICS()
        : visits(0), state(new STATE())
    {
        memset(results, 0, sizeof(uint32_t) * 3);
    }

    STATISTICS(STATE state)
        : visits(0), state(state)
    {
        memset(results, 0, sizeof(uint32_t) * 3);
    }

    STATISTICS(STATISTICS* source)
        : state(STATE(source->state)), visits(source->visits)
    {
        memcpy(results, source->results, sizeof(uint32_t) * 3);
    }
};

class NODE
{
public:
    NODE* parent;
    uint16_t parent_action;
    STATISTICS* data;
    std::list<NODE*> children;
    std::vector<uint16_t> untried_actions;
#ifdef RAVE
    uint32_t RAVE_visits;
    uint32_t RAVE_local_counts[SIZE * SIZE];
    uint32_t RAVE_local_results[SIZE * SIZE][3];
#endif

    NODE()
        : parent(nullptr), data(new STATISTICS())
#ifdef RAVE
        , RAVE_visits(0)
#endif
    {
        untried_actions = data->state.possible();
        std::shuffle(std::begin(untried_actions), std::end(untried_actions), rng);
#ifdef RAVE
        memset(RAVE_local_counts, 0, sizeof(uint32_t) * SIZE * SIZE);
        memset(RAVE_local_results, 0, sizeof(uint32_t) * SIZE * SIZE * 3);
#endif
    }

    NODE(STATE state)
        : parent(nullptr), data(new STATISTICS(state))
#ifdef RAVE
        , RAVE_visits(0)
#endif
    {
        untried_actions = data->state.possible();
        std::shuffle(std::begin(untried_actions), std::end(untried_actions), rng);
#ifdef RAVE
        memset(RAVE_local_counts, 0, sizeof(uint32_t) * SIZE * SIZE);
        memset(RAVE_local_results, 0, sizeof(uint32_t) * SIZE * SIZE * 3);
#endif
    }

    NODE(STATE state, NODE* parent, uint16_t parent_action)
        : parent(parent), parent_action(parent_action), data(new STATISTICS(state))
#ifdef RAVE
        , RAVE_visits(0)
#endif
    {
        untried_actions = data->state.possible();
        std::shuffle(std::begin(untried_actions), std::end(untried_actions), rng);
#ifdef RAVE
        memset(RAVE_local_counts, 0, sizeof(uint32_t) * SIZE * SIZE);
        memset(RAVE_local_results, 0, sizeof(uint32_t) * SIZE * SIZE * 3);
#endif
    }

    NODE(STATISTICS* data, NODE* parent, uint16_t parent_action)
        : parent(parent), parent_action(parent_action), data(data)
#ifdef RAVE
        , RAVE_visits(0)
#endif
    {
        untried_actions = data->state.possible();
        std::shuffle(std::begin(untried_actions), std::end(untried_actions), rng);
#ifdef RAVE
        memset(RAVE_local_counts, 0, sizeof(uint32_t) * SIZE * SIZE);
        memset(RAVE_local_results, 0, sizeof(uint32_t) * SIZE * SIZE * 3);
#endif
    }

    NODE(NODE* source)
        : parent(source->parent), parent_action(source->parent_action), data(new STATISTICS(source->data))
#ifdef RAVE
        , RAVE_visits(source->RAVE_visits)
#endif
    {
        untried_actions.reserve(source->untried_actions.size());
        untried_actions.assign(source->untried_actions.begin(), source->untried_actions.end());
#ifdef RAVE
        memcpy(RAVE_local_counts, source->RAVE_local_counts, sizeof(uint32_t) * SIZE * SIZE);
        memcpy(RAVE_local_results, source->RAVE_local_results, sizeof(uint32_t) * SIZE * SIZE * 3);
#endif
        for (NODE* child : source->children) children.push_back(new NODE(child));
    }

    NODE(NODE* source, NODE* parent)
        : parent(parent), parent_action(source->parent_action), data(new STATISTICS(source->data))
#ifdef RAVE
        , RAVE_visits(source->RAVE_visits)
#endif
    {
        untried_actions.reserve(source->untried_actions.size());
        untried_actions.assign(source->untried_actions.begin(), source->untried_actions.end());
#ifdef RAVE
        memcpy(RAVE_local_counts, source->RAVE_local_counts, sizeof(uint32_t) * SIZE * SIZE);
        memcpy(RAVE_local_results, source->RAVE_local_results, sizeof(uint32_t) * SIZE * SIZE * 3);
#endif
        for (NODE* child : source->children) children.push_back(new NODE(child));
    }

    ~NODE()
    {
        for (NODE* child : children) delete child;
    }

    NODE* expand()
    {
        uint16_t index = untried_actions.back();
        untried_actions.pop_back();

        STATE resulting_state(data->state);
        resulting_state.action(index);

        NODE* child;
        STATISTICS* child_stats;
        auto TT_stats = TT->find(resulting_state.hash_value);

        if (TT_stats != TT->end())
        {
            TT_hits++;
            child_stats = TT_stats->second;
            //for (uint16_t i = 0; i < SIZE; i++)
            //    if ((child_stats->state.c_array[i] != resulting_state.c_array[i]) || (child_stats->state.m_array[i] != resulting_state.m_array[i]))
            //    {
            //        std::cout << "Hash collision occured\n";
            //        child_stats->state.render();
            //        std::cout << "vs\n";
            //        resulting_state.render();
            //        std::cout << "----------------------------------------\n";
            //        throw("Hash collision occured");
            //    }
            child = new NODE(child_stats, this, index);
            children.push_back(child);
            return this->policy();
        }

        child_stats = new STATISTICS(resulting_state);
        child = new NODE(child_stats, this, index);
        TT->insert({ resulting_state.hash_value, child_stats });
        children.push_back(child);

        return child;
    }

    void rollout()
    {
        STATE simulation_state = STATE(data->state);
        std::uniform_int_distribution<std::mt19937::result_type> distribution(0, untried_actions.size());
        uint16_t index = distribution(rng);

        while (!simulation_state.terminal())
        {
            simulation_state.action(untried_actions[index % untried_actions.size()]);
            index++;
        }
#ifdef RAVE
        backpropagate(simulation_state.result, parent_action);
#else
        backpropagate(simulation_state.result);
#endif
    }

#ifdef RAVE
    void backpropagate(uint8_t value, uint16_t action)
    {
        data->visits++;
        data->results[value]++;
        RAVE_visits++;
        RAVE_local_counts[action]++;
        RAVE_local_results[action][value]++;
        if (parent)
            parent->backpropagate(value, action);
    }
#else
    void backpropagate(uint8_t value)
    {
        data->visits++;
        data->results[value]++;
        if (parent)
            parent->backpropagate(value);
    }
#endif
#ifdef RAVE
    int32_t RAVE_delta(uint16_t action, bool turn)
    {
        if (turn)   return RAVE_local_results[action][0] - RAVE_local_results[action][1];
        else        return RAVE_local_results[action][1] - RAVE_local_results[action][0];
    }
#endif
    int32_t Q_delta(bool turn)
    {
        if (turn)   return data->results[0] - data->results[1];
        else        return data->results[1] - data->results[0];
    }

    NODE* best_child()
    {
        NODE* best_child = nullptr;
        PREC best_result = -100.0;

        // Precompute
        PREC log_visits = 2 * log_table[data->visits];
        bool turn = data->state.empty % 2;

        PREC Q_value;
        PREC RAVE_score;

        PREC result;
        for (NODE* child : children)
        {

            Q_value = PREC(child->Q_delta(turn)) / PREC(child->data->visits);
#ifdef RAVE
            RAVE_score = (1 - RAVE_BIAS) * Q_value + RAVE_BIAS * (PREC(child->RAVE_delta(child->parent_action, turn)) / PREC(RAVE_local_counts[child->parent_action]));
            result = Q_value + BIAS * std::sqrt(log_visits / PREC(child->data->visits))
                + RAVE_BIAS * std::sqrt(log_table[child->RAVE_local_counts[child->parent_action]] / PREC(child->RAVE_visits)) + RAVE_score;
#else
            result = Q_value + BIAS * std::sqrt(log_visits / PREC(child->data->visits));
#endif
            if (result > best_result)
            {
                best_result = result;
                best_child = child;
            }
        }
        return best_child;
    }

    NODE* policy()
    {
        NODE* current = this;
        while (!current->data->state.terminal())
            if (current->untried_actions.size() > 0)
                return current->expand();
            else
                current = current->best_child();
        TT_subcheck++;
        return current;
    }
};

class HOST
{
public:

    static void init()
    {
        for (uint32_t i = 1; i < BATCH_SIZE; i++)
            log_table[i] = std::log(i);
        log_table_size = BATCH_SIZE;

        std::uniform_int_distribution<int64_t> distribution;
        for (int i = 0; i < SIZE * SIZE; ++i)
            for (int j = 0; j < 3; ++j)
                zobrist_table[i][j] = distribution(rng);
    }

    static STATE* create()
    {
        return new STATE();
    }

    static STATE* create(std::string position)
    {
        STATE* state = new STATE();
        for (int i = 0; i < position.length(); i += 2)
        {
            uint16_t x = position[i] - '0';
            uint16_t y = position[i + 1] - '0';
            state->action(y * SIZE + x);
        }
        return state;
    }

    static void human_move(STATE* state)
    {
        bool getting_input = true;
        std::string input_x;
        std::string input_y;
        uint16_t x;
        uint16_t y;
        uint16_t index;
        while (getting_input)
        {
            try
            {
                std::cout << "Action X: ";
                std::cin >> input_x;
                std::cout << "Action Y: ";
                std::cin >> input_y;
                x = stoi(input_x);
                y = stoi(input_y);
            }
            catch (const std::exception& e)
            {
                std::cout << "Invalid input format!\n";
                continue;
            }
            index = y * SIZE + x;
            if ((0 <= x && x < SIZE) && (0 <= y && y < SIZE))
            {
                if (!(state->m_array[y] & (BLOCK(1) << x % SIZE)))
                    getting_input = false;
                else
                    std::cout << "Selected field is occupied!\n";
            }
            else
                std::cout << "Selection outside the field!\n";
        }
        state->action(index);
    }

    static void MCTS_move(STATE* root_state, std::chrono::milliseconds time, PREC confidence_bound, bool analytics)
    {
        NODE* root = new NODE(root_state);
        // Build Tree
        uint64_t i;
        uint64_t ix = 0;
        auto start = std::chrono::high_resolution_clock::now();
        while (std::chrono::high_resolution_clock::now() - start < time)
        {
            for (i = 0; i < BATCH_SIZE; i++)
            {
                NODE* node = root->policy();
                node->rollout();
            }
            ix++;
            update(ix);
        }
        MCTS_master(root, root_state, confidence_bound, analytics);
    }

    static void MCTS_move(STATE* root_state, uint64_t simulations, PREC confidence_bound, bool analytics)
    {
        NODE* root = new NODE(root_state);
        for (uint64_t i = log_table_size; i < simulations; i++)
            log_table[i] = std::log(i);
        if (log_table_size < simulations)
            log_table_size = simulations;
        // Build Tree
        for (uint64_t i = 0; i < simulations; i++)
        {
            NODE* node = root->policy();
            node->rollout();
        }
        MCTS_master(root, root_state, confidence_bound, analytics);
    }

    static void render_sim_distribution(NODE* root)
    {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
        std::cout << "\n    <";
        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << "-";
        std::cout << " SIMULATIONS DISTRIBUTION ";

        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << "-";
        std::cout << ">\n";

        PREC max_visits = 0;
        for (NODE* child : root->children)
            if (child->data->visits > max_visits)
                max_visits = child->data->visits;

        std::cout << "    ";
        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << " " << std::to_string(i).append(3 - std::to_string(i).length(), ' ');
        std::cout << "\n   ";
        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << " ---";
        std::cout << "\n";
        for (uint16_t y = 0; y < SIZE; y++)
        {
            std::cout << std::to_string(y).append(3 - std::to_string(y).length(), ' ');
            for (uint16_t x = 0; x < SIZE; x++)
            {
                std::cout << "|";
                if (!(root->data->state.m_array[y] & (BLOCK(1) << x)))
                {
                    for (NODE* child : root->children)
                        if (child->parent_action == (y * SIZE + x))
                            std::printf("%3d", int(PREC(child->data->visits) / max_visits * 100));
                }

                else if (root->data->state.c_array[y] & (BLOCK(1) << x))
                {
#ifdef _WIN32
                    SetConsoleTextAttribute(hConsole, root->data->state.empty % 2 ? 9 : 4);
                    std::cout << " o ";
                    SetConsoleTextAttribute(hConsole, 7);
#else
                    if (!(root->state.empty % 2))   std::cout << "\033[1;34m o \033[0m";
                    else                std::cout << "\033[1;31m o \033[0m";
#endif
                }
                else if (!(root->data->state.c_array[y] & (BLOCK(1) << x)))
                {
#ifdef _WIN32
                    SetConsoleTextAttribute(hConsole, root->data->state.empty % 2 ? 4 : 9);
                    std::cout << " o ";
                    SetConsoleTextAttribute(hConsole, 7);
#else
                    if (root->state.empty % 2)      std::cout << "\033[1;34m o \033[0m";
                    else                std::cout << "\033[1;31m o \033[0m";
#endif
                }
            }
            std::cout << "|\n   ";
            for (uint16_t i = 0; i < SIZE; i++)
                std::cout << " ---";
            std::cout << "\n";
        }

        std::cout << "    <";
        for (uint16_t i = 0; i < SIZE * 2 + 26; i++)
            std::cout << "-";
        std::cout << ">\n";
    }

    static void render_ucb_distribution(NODE* root)
    {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
        std::cout << "\n    <";
        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << "-";
        std::cout << "     UCB DISTRIBUTION     ";

        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << "-";
        std::cout << ">\n";

        std::cout << "    ";
        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << " " << std::to_string(i).append(3 - std::to_string(i).length(), ' ');
        std::cout << "\n   ";
        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << " ---";
        std::cout << "\n";
        for (uint16_t y = 0; y < SIZE; y++)
        {
            std::cout << std::to_string(y).append(3 - std::to_string(y).length(), ' ');
            for (uint16_t x = 0; x < SIZE; x++)
            {
                std::cout << "|";
                if (!(root->data->state.m_array[y] & (BLOCK(1) << x)))
                {
                    for (NODE* child : root->children)
                        if (child->parent_action == (y * SIZE + x))
                            std::printf("%+3d", int(PREC(child->Q_delta(root->data->state.empty % 2)) / PREC(child->data->visits) * 100));
                }

                else if (root->data->state.c_array[y] & (BLOCK(1) << x))
                {
#ifdef _WIN32
                    SetConsoleTextAttribute(hConsole, root->data->state.empty % 2 ? 9 : 4);
                    std::cout << " o ";
                    SetConsoleTextAttribute(hConsole, 7);
#else
                    if (!(root->state.empty % 2))   std::cout << "\033[1;34m o \033[0m";
                    else                std::cout << "\033[1;31m o \033[0m";
#endif
                }
                else if (!(root->data->state.c_array[y] & (BLOCK(1) << x)))
                {
#ifdef _WIN32
                    SetConsoleTextAttribute(hConsole, root->data->state.empty % 2 ? 4 : 9);
                    std::cout << " o ";
                    SetConsoleTextAttribute(hConsole, 7);
#else
                    if (root->state.empty % 2)      std::cout << "\033[1;34m o \033[0m";
                    else                std::cout << "\033[1;31m o \033[0m";
#endif
                }
            }
            std::cout << "|\n   ";
            for (uint16_t i = 0; i < SIZE; i++)
                std::cout << " ---";
            std::cout << "\n";
        }

        std::cout << "    <";
        for (uint16_t i = 0; i < SIZE * 2 + 26; i++)
            std::cout << "-";
        std::cout << ">\n";
    }
#ifdef RAVE
    static void render_RAVE_distribution(NODE* root)
    {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
        std::cout << "\n    <";
        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << "-";
        std::cout << "    RAVE DISTRIBUTION     ";

        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << "-";
        std::cout << ">\n";

        std::cout << "    ";
        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << " " << std::to_string(i).append(3 - std::to_string(i).length(), ' ');
        std::cout << "\n   ";
        for (uint16_t i = 0; i < SIZE; i++)
            std::cout << " ---";
        std::cout << "\n";
        for (uint16_t y = 0; y < SIZE; y++)
        {
            std::cout << std::to_string(y).append(3 - std::to_string(y).length(), ' ');
            for (uint16_t x = 0; x < SIZE; x++)
            {
                std::cout << "|";
                if (!(root->data->state.m_array[y] & (BLOCK(1) << x)))
                {
                    std::printf("%+3d", int(PREC(root->RAVE_delta(y * SIZE + x, root->data->state.empty % 2)) / PREC(root->RAVE_local_counts[y * SIZE + x]) * 100));
                }

                else if (root->data->state.c_array[y] & (BLOCK(1) << x))
                {
#ifdef _WIN32
                    SetConsoleTextAttribute(hConsole, root->data->state.empty % 2 ? 9 : 4);
                    std::cout << " o ";
                    SetConsoleTextAttribute(hConsole, 7);
#else
                    if (!(empty % 2))   std::cout << "\033[1;34m o \033[0m";
                    else                std::cout << "\033[1;31m o \033[0m";
#endif
                }
                else if (!(root->data->state.c_array[y] & (BLOCK(1) << x)))
                {
#ifdef _WIN32
                    SetConsoleTextAttribute(hConsole, root->data->state.empty % 2 ? 4 : 9);
                    std::cout << " o ";
                    SetConsoleTextAttribute(hConsole, 7);
#else
                    if (empty % 2)      std::cout << "\033[1;34m o \033[0m";
                    else                std::cout << "\033[1;31m o \033[0m";
#endif
                }
            }
            std::cout << "|\n   ";
            for (uint16_t i = 0; i < SIZE; i++)
                std::cout << " ---";
            std::cout << "\n";
        }

        std::cout << "    <";
        for (uint16_t i = 0; i < SIZE * 2 + 26; i++)
            std::cout << "-";
        std::cout << ">\n";
    }
#endif
private:
    static void update(uint64_t ix)
    {

        uint64_t i = log_table_size;
        for (i; i < (ix + 1) * BATCH_SIZE; i++)
            log_table[i] = std::log(i);
        if (i > log_table_size)
            log_table_size = i;
    }

    static void MCTS_master(NODE* root, STATE* root_state, PREC confidence_bound, bool analytics)
    {
        // Select best child
        NODE* best = nullptr;
        std::list<NODE*> children;
        for (NODE* child : root->children) children.push_back(child);
        uint16_t children_count = root->children.size();
        for (uint16_t i = 0; i < children_count; i++)
        {
            NODE* child = best_child_final(root);
            if (PREC(child->data->visits) / PREC(root->data->visits) > confidence_bound)
            {
                best = child;
                break;
            }
            root->children.remove(child);
        }
        root->children = children;

        if (!best)
        {
            std::cout << "[WARNING]: No action in confidence bound!\n";
            best = best_child_final(root);
        }

        print_evaluation(best);
        if (analytics)
        {
            render_sim_distribution(root);
            render_ucb_distribution(root);
#ifdef RAVE
            render_RAVE_distribution(root);
#endif
        }

        STATE* best_state = new STATE(best->data->state);
        root_state->action(best->parent_action);

        delete root;
        for (auto& [key, value] : (*TT))
            delete value;

        TT->clear();
        TT_subcheck = 0;
        TT_hits = 0;
    }

    static void print_evaluation(NODE* best)
    {
        std::cout << "Action:      " << int32_t(best->parent_action) % SIZE << "," << int32_t(best->parent_action) / SIZE << "\n";
        std::cout << "Simulations: " << PREC(int32_t(best->parent->data->visits) / 1000) / 1000 << "M";
        std::cout << " (W:" << int(best->parent->data->results[best->parent->data->state.empty % 2 ? 0 : 1]) << " L:" << int(best->parent->data->results[best->parent->data->state.empty % 2 ? 1 : 0]) << " D:" << int(best->parent->data->results[2]) << ")\n";
        std::cout << "Evaluation:  " << PREC(PREC(best->Q_delta(best->parent->data->state.empty % 2)) / PREC(best->data->visits));
        std::cout << " (W:" << int(best->data->results[best->parent->data->state.empty % 2 ? 0 : 1]) << " L:" << int(best->data->results[best->parent->data->state.empty % 2 ? 1 : 0]) << " D:" << int(best->data->results[2]) << ")\n";
        std::cout << "TT-Hitrate:  " << PREC(TT_hits * 100) / PREC(best->parent->data->visits) << "%\n";
        int32_t TT_size = TT->size();
        if (TT_size != (best->parent->data->visits - TT_subcheck))
            std::cout << "[WARNING]: Hash-collision\nTT.size() -> " << TT_size << "\nExpected -> " << best->parent->data->visits - TT_subcheck << "\n";
        std::cout << "Confidence:  " << PREC(best->data->visits * 100) / PREC(best->parent->data->visits) << "%\n";
        std::printf("Draw:        %.2f%%\n", 2, PREC(best->data->results[2] * 100) / PREC(best->data->visits));
    }

    static NODE* best_child_final(NODE* node)
    {
        NODE* best_child = nullptr;
        PREC result;
        PREC best_result = -100.0;

        // Precompute
        bool turn = node->data->state.empty % 2;

        for (NODE* child : node->children)
        {
            PREC Q_value = PREC(child->Q_delta(turn)) / PREC(child->data->visits);
#ifdef RAVE
            PREC RAVE_score = (1 - RAVE_BIAS) * Q_value + RAVE_BIAS * (PREC(node->RAVE_delta(child->parent_action, node->data->state.empty % 2)) / PREC(node->RAVE_local_counts[child->parent_action]));
            result = Q_value + RAVE_score;
#else
            result = Q_value;
#endif
            if (result > best_result)
            {
                best_result = result;
                best_child = child;
            }
        }
        return best_child;
    }
};

int main()
{
    HOST::init();

    STATE* state = HOST::create();

    state->render();
    while (!state->terminal())
    {
        if (!(state->empty % 2))
            HOST::MCTS_move(state, std::chrono::seconds(10), 0.01, true);
        //HOST::human_move(state);  
        else
            HOST::human_move(state);
        state->render();
    }
}