#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

static bool isSpaceChar(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static bool isDigitChar(char c)
{
    return c >= '0' && c <= '9';
}

class Turn
{
public:
    static const Turn User;
    static const Turn Computer;

    bool operator==(const Turn& other) const
    {
        return v == other.v;
    }

    bool operator!=(const Turn& other) const
    {
        return v != other.v;
    }

private:
    explicit Turn(int value) : v(value) {}
    int v;
};

const Turn Turn::User(0);
const Turn Turn::Computer(1);

class GameNode
{
public:
    char board[9];
    Turn toMove;
    int score;
    vector<GameNode*> children;

    GameNode(const char src[9], Turn t);
    ~GameNode();
};

GameNode::GameNode(const char src[9], Turn t) : toMove(t), score(0), children()
{
    for (int i = 0; i < 9; i++)
    {
        board[i] = src[i];
    }
}

GameNode::~GameNode()
{
    for (GameNode* c : children)
    {
        delete c;
    }
}

static char cellDisplay(char c)
{
    return (c == 'b') ? ' ' : c;
}

void printBoard(const char b[9])
{
    cout << "+---+---+---+\n";
    for (int r = 0; r < 3; r++)
    {
        cout << "|";
        for (int c = 0; c < 3; c++)
        {
            cout << ' ' << cellDisplay(b[r * 3 + c]) << " |";
        }
        cout << '\n';
        cout << "+---+---+---+\n";
    }
}

// Returns true if line complete for sym
static bool lineWin(const char b[9], char sym, int a, int x, int y)
{
    return b[a] == sym && b[x] == sym && b[y] == sym;
}

// If terminal, sets score: +1 o wins, -1 x wins, 0 draw. Returns true if terminal.
static bool terminalScore(const char b[9], int& scoreOut)
{
    const int lines[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7},
                             {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};
    for (int k = 0; k < 2; k++)
    {
        char sym = (k == 0) ? 'x' : 'o';
        int s = (sym == 'o') ? 1 : -1;
        for (int L = 0; L < 8; L++)
        {
            if (lineWin(b, sym, lines[L][0], lines[L][1], lines[L][2]))
            {
                scoreOut = s;
                return true;
            }
        }
    }
    bool full = true;
    for (int i = 0; i < 9; i++)
    {
        if (b[i] == 'b')
        {
            full = false;
            break;
        }
    }
    if (full)
    {
        scoreOut = 0;
        return true;
    }
    return false;
}

// Recursive minimax: builds children, assigns scores. Returns node->score.
static int evaluateNode(GameNode* node)
{
    int leaf = 0;
    if (terminalScore(node->board, leaf))
    {
        node->score = leaf;
        return leaf;
    }

    char sym = (node->toMove == Turn::Computer) ? 'o' : 'x';
    Turn next = (node->toMove == Turn::Computer) ? Turn::User : Turn::Computer;

    for (int i = 0; i < 9; i++)
    {
        if (node->board[i] != 'b')
        {
            continue;
        }
        GameNode* child = new GameNode(node->board, next);
        child->board[i] = sym;
        node->children.push_back(child);
    }

    if (node->toMove == Turn::Computer)
    {
        int best = -2;
        for (GameNode* c : node->children)
        {
            int v = evaluateNode(c);
            if (v > best)
            {
                best = v;
            }
        }
        node->score = best;
    }
    else
    {
        int worst = 2;
        for (GameNode* c : node->children)
        {
            int v = evaluateNode(c);
            if (v < worst)
            {
                worst = v;
            }
        }
        node->score = worst;
    }
    return node->score;
}

// Index of the cell that differs between a and b (exactly one change); -1 if invalid.
static int diffMoveIndex(const char a[9], const char b[9])
{
    int idx = -1;
    for (int i = 0; i < 9; i++)
    {
        if (a[i] != b[i])
        {
            if (idx != -1)
            {
                return -1;
            }
            idx = i;
        }
    }
    return idx;
}

static int computerChooseMove(char b[9])
{
    GameNode* root = new GameNode(b, Turn::Computer);
    evaluateNode(root);

    int bestRating = -2;
    vector<int> ties;
    for (GameNode* child : root->children)
    {
        int mv = diffMoveIndex(b, child->board);
        if (mv < 0)
        {
            continue;
        }
        if (child->score > bestRating)
        {
            bestRating = child->score;
            ties.clear();
            ties.push_back(mv);
        }
        else if (child->score == bestRating)
        {
            ties.push_back(mv);
        }
    }

    int n = static_cast<int>(ties.size());
    int pick = (n <= 0) ? 0 : ties[rand() % n];

    delete root;
    return pick;
}

static bool readIntInRange(const string& line, int& out)
{
    if (line.empty())
    {
        return false;
    }
    size_t i = 0;
    while (i < line.size() && isSpaceChar(line[i]))
    {
        i++;
    }
    if (i == line.size())
    {
        return false;
    }
    int sign = 1;
    if (line[i] == '-')
    {
        sign = -1;
        i++;
        if (i == line.size())
        {
            return false;
        }
    }
    if (!isDigitChar(line[i]))
    {
        return false;
    }
    long val = 0;
    for (; i < line.size() && isDigitChar(line[i]); i++)
    {
        val = val * 10 + (line[i] - '0');
        if (val > 20)
        {
            break;
        }
    }
    val *= sign;
    if (val < 0 || val > 8)
    {
        return false;
    }
    while (i < line.size())
    {
        if (!isSpaceChar(line[i]))
        {
            return false;
        }
        i++;
    }
    out = static_cast<int>(val);
    return true;
}

static int promptUserMove(const char b[9])
{
    string line;
    int idx;
    for (;;)
    {
        cout << "Enter your move (0-8): ";
        if (!getline(cin, line))
        {
            return -1;
        }
        if (!readIntInRange(line, idx))
        {
            cout << "Invalid input. Use an integer 0-8 for an empty cell.\n";
            continue;
        }
        if (b[idx] != 'b')
        {
            cout << "That square is not empty. Try again.\n";
            continue;
        }
        return idx;
    }
}

int main()
{
    srand(static_cast<unsigned>(time(nullptr)));

    char b[9];
    for (int i = 0; i < 9; i++)
    {
        b[i] = 'b';
    }

    cout << "Who starts?\n";
    cout << "  1 - You (x)\n";
    cout << "  2 - Computer (o)\n";
    string line;
    Turn current = Turn::User;
    for (;;)
    {
        cout << "Choice (1 or 2): ";
        if (!getline(cin, line))
        {
            return 0;
        }
        int ch;
        if (readIntInRange(line, ch) && (ch == 1 || ch == 2))
        {
            current = (ch == 1) ? Turn::User : Turn::Computer;
            break;
        }
        cout << "Please enter 1 or 2.\n";
    }

    cout << "\nInitial board:\n";
    printBoard(b);
    cout << '\n';

    for (;;)
    {
        int sc = 0;
        if (terminalScore(b, sc))
        {
            break;
        }

        if (current == Turn::User)
        {
            int mv = promptUserMove(b);
            if (mv < 0)
            {
                return 0;
            }
            b[mv] = 'x';
            cout << "\nAfter your move:\n";
            printBoard(b);
            cout << '\n';
            current = Turn::Computer;
        }
        else
        {
            int mv = computerChooseMove(b);
            b[mv] = 'o';
            cout << "Computer plays " << mv << ".\n";
            cout << "\nAfter computer move:\n";
            printBoard(b);
            cout << '\n';
            current = Turn::User;
        }
    }

    int finalScore = 0;
    terminalScore(b, finalScore);
    if (finalScore == 1)
    {
        cout << "Computer wins.\n";
    }
    else if (finalScore == -1)
    {
        cout << "User wins.\n";
    }
    else
    {
        cout << "Draw.\n";
    }
    return 0;
}
