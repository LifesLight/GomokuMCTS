#include "Definitions.h"
#include "Statistics.h"
#include "State.h"
#include "Config.h"
#include "Node.h"

class HOST
{
public:
    static void init()
    {
        for (uint32_t i = 1; i < MaxSimulations; i++)
            logTable[i] = std::log(i);
        
        std::uniform_int_distribution<int64_t> distribution;
        for (int i = 0; i < BoardSize * BoardSize; ++i)
            for (int j = 0; j < 3; ++j)
                zobrist_table[i][j] = distribution(rng);
    }

    static State* create()
    {
        return new State();
    }

    static State* create(std::string position)
    {
        State* state = new State();
        for (int i = 0; i < position.length(); i += 2)
        {
            uint16_t x = position[i] - '0';
            uint16_t y = position[i + 1] - '0';
            state->action(y * BoardSize + x);
        }
        return state;
    }

    static void human_move(State* state)
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
            index = y * BoardSize + x;
            if ((0 <= x && x < BoardSize) && (0 <= y && y < BoardSize))
            {
                if (!(state->m_array[y] & (BLOCK(1) << x % BoardSize)))
                    getting_input = false;
                else
                    std::cout << "Selected field is occupied!\n";
            }
            else
                std::cout << "Selection outside the field!\n";
        }
        state->action(index);
    }

    static void MCTS_move(State* root_state, std::chrono::milliseconds time, FloatPrecision confidence_bound, bool analytics)
    {
        Node* root = new Node(root_state);
        // Build Tree
        uint64_t i;
        auto start = std::chrono::high_resolution_clock::now();
        while (std::chrono::high_resolution_clock::now() - start < time)
        {
            for (i = 0; i < 1000; i++)
            {
                Node* node = root->policy();
                node->rollout();
            }
        }
        MCTS_master(root, root_state, confidence_bound, analytics);
    }

    static void MCTS_move(State* root_state, uint64_t simulations, FloatPrecision confidence_bound, bool analytics)
    {
        Node* root = new Node(root_state);
        // Build Tree
        for (uint64_t i = 0; i < simulations; i++)
        {
            Node* node = root->policy();
            node->rollout();
        }
        MCTS_master(root, root_state, confidence_bound, analytics);
    }


static std::string sim_distribution(Node* root)
{
    std::ostringstream result;

    result << "\n    <";
    for (uint16_t i = 0; i < BoardSize; i++)
        result << "-";
    result << " SIMULATIONS DISTRIBUTION ";

    for (uint16_t i = 0; i < BoardSize; i++)
        result << "-";
    result << ">\n";

    FloatPrecision max_visits = 0;
    for (Node* child : root->children)
        if (child->data->visits > max_visits)
            max_visits = child->data->visits;

    result << "    ";
    for (uint16_t i = 0; i < BoardSize; i++)
        result << " " << std::setw(3) << std::setfill(' ') << i;
    result << "\n   ";
    for (uint16_t i = 0; i < BoardSize; i++)
        result << " ---";
    result << "\n";
    for (uint16_t y = 0; y < BoardSize; y++)
    {
        result << std::setw(3) << std::setfill(' ') << y;
        for (uint16_t x = 0; x < BoardSize; x++)
        {
            result << "|";
            if (!(root->data->state.m_array[y] & (BLOCK(1) << x)))
            {
                for (Node* child : root->children)
                    if (child->parent_action == (y * BoardSize + x))
                        result << std::setw(3) << std::setfill(' ') << int(FloatPrecision(child->data->visits) / max_visits * 100);
            }
            else if (root->data->state.c_array[y] & (BLOCK(1) << x))
            {
                if (!(root->data->state.empty % 2))
                    result << "\033[1;34m o \033[0m";
                else
                    result << "\033[1;31m o \033[0m";
            }
            else if (!(root->data->state.c_array[y] & (BLOCK(1) << x)))
            {
                if (root->data->state.empty % 2)
                    result << "\033[1;34m o \033[0m";
                else
                    result << "\033[1;31m o \033[0m";
            }
        }
        result << "|\n   ";
        for (uint16_t i = 0; i < BoardSize; i++)
            result << " ---";
        result << "\n";
    }

    result << "    <";
    for (uint16_t i = 0; i < BoardSize * 2 + 26; i++)
        result << "-";
    result << ">\n";

    return result.str();
}


static std::string ucb_distribution(Node* root)
{
    std::ostringstream result;

    result << "\n    <";
    for (uint16_t i = 0; i < BoardSize; i++)
        result << "-";
    result << "     UCB DISTRIBUTION     ";

    for (uint16_t i = 0; i < BoardSize; i++)
        result << "-";
    result << ">\n";

    result << "    ";
    for (uint16_t i = 0; i < BoardSize; i++)
        result << " " << std::setw(3) << std::setfill(' ') << i;
    result << "\n   ";
    for (uint16_t i = 0; i < BoardSize; i++)
        result << " ---";
    result << "\n";
    for (uint16_t y = 0; y < BoardSize; y++)
    {
        result << std::setw(3) << std::setfill(' ') << y;
        for (uint16_t x = 0; x < BoardSize; x++)
        {
            result << "|";
            if (!(root->data->state.m_array[y] & (BLOCK(1) << x)))
            {
                for (Node* child : root->children)
                    if (child->parent_action == (y * BoardSize + x))
                        result << std::setw(3) << std::setfill(' ') << int(FloatPrecision(child->qDelta(root->data->state.empty % 2)) / FloatPrecision(child->data->visits) * 100);
            }
            else if (root->data->state.c_array[y] & (BLOCK(1) << x))
            {
                if (!(root->data->state.empty % 2))
                    result << "\033[1;34m o \033[0m";
                else
                    result << "\033[1;31m o \033[0m";
            }
            else if (!(root->data->state.c_array[y] & (BLOCK(1) << x)))
            {
                if (root->data->state.empty % 2)
                    result << "\033[1;34m o \033[0m";
                else
                    result << "\033[1;31m o \033[0m";
            }
        }
        result << "|\n   ";
        for (uint16_t i = 0; i < BoardSize; i++)
            result << " ---";
        result << "\n";
    }

    result << "    <";
    for (uint16_t i = 0; i < BoardSize * 2 + 26; i++)
        result << "-";
    result << ">\n";

    return result.str();
}



private:
    static void MCTS_master(Node* root, State* root_state, FloatPrecision confidence_bound, bool analytics)
    {
        // Select best child
        Node* best = nullptr;
        std::list<Node*> children;
        for (Node* child : root->children) children.push_back(child);
        uint16_t children_count = root->children.size();
        for (uint16_t i = 0; i < children_count; i++)
        {
            Node* child = best_child_final(root);
            if (FloatPrecision(child->data->visits) / FloatPrecision(root->data->visits) > confidence_bound)
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
        
        if (analytics)
        {
            std::cout << sim_distribution(root);
            std::cout << ucb_distribution(root);
        }

        print_evaluation(best);

        State* best_state = new State(best->data->state);
        root_state->action(best->parent_action);

        delete root;
        for (auto& [key, value] : (*TT))
            delete value;

        TT->clear();
        TT_hits = 0;
    }

    static void print_evaluation(Node* best)
    {
        std::cout << "Action:      " << int32_t(best->parent_action) % BoardSize << "," << int32_t(best->parent_action) / BoardSize << "\n";
        std::cout << "Simulations: " << FloatPrecision(int32_t(best->parent->data->visits) / 1000) / 1000 << "M";
        std::cout << " (W:" << int(best->parent->data->results[best->parent->data->state.empty % 2 ? 0 : 1]) << " L:" << int(best->parent->data->results[best->parent->data->state.empty % 2 ? 1 : 0]) << " D:" << int(best->parent->data->results[2]) << ")\n";
        std::cout << "Evaluation:  " << FloatPrecision(FloatPrecision(best->qDelta(best->parent->data->state.empty % 2)) / FloatPrecision(best->data->visits));
        std::cout << " (W:" << int(best->data->results[best->parent->data->state.empty % 2 ? 0 : 1]) << " L:" << int(best->data->results[best->parent->data->state.empty % 2 ? 1 : 0]) << " D:" << int(best->data->results[2]) << ")\n";
        std::cout << "Confidence:  " << FloatPrecision(best->data->visits * 100) / FloatPrecision(best->parent->data->visits) << "%\n";
        std::cout << "TT-Hitrate:  " << FloatPrecision(TT_hits * 100) / FloatPrecision(best->parent->data->visits) << "%\n";
        std::printf("Draw:        %.2f%%\n", FloatPrecision(best->data->results[2] * 100) / FloatPrecision(best->data->visits));

        std::cout << "    <";
        for (uint16_t i = 0; i < BoardSize * 2 + 26; i++)
            std::cout << "-";
        std::cout << ">\n";
    }

    static Node* best_child_final(Node* node)
    {
        Node* best_child = nullptr;
        FloatPrecision result;
        FloatPrecision best_result = -100.0;

        // Precompute
        bool turn = node->data->state.empty % 2;

        for (Node* child : node->children)
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
};

int main()
{
    HOST::init();

    State* state = HOST::create();

    std::cout << state->toString();
    while (!state->terminal())
    {
        if (!(state->empty % 2))
            HOST::MCTS_move(state, 1'000'000, 0.01, true);
        else
            HOST::human_move(state);
        std::cout << state->toString();
    }
}
