// Added UI to the base code to make it more user friendly and interactive.

#include <windows.h>
#include <windowsx.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

namespace
{
    constexpr int kWindowWidth = 420;
    constexpr int kWindowHeight = 520;

    constexpr int kBoardSize = 360;
    constexpr int kCellSize = kBoardSize / 3;
    constexpr int kBoardLeft = 30;
    constexpr int kBoardTop = 30;

    constexpr int kStatusTop = 410;
    constexpr int kButtonTop = 450;
    constexpr int kButtonLeft = 145;
    constexpr int kButtonWidth = 130;
    constexpr int kButtonHeight = 38;

    enum class GameState
    {
        Playing,
        XWon,
        OWon,
        Draw
    };

    class Turn
    {
    public:
        static const Turn User;
        static const Turn Computer;

        bool operator==(const Turn& other) const { return v == other.v; }
        bool operator!=(const Turn& other) const { return v != other.v; }

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
        std::vector<GameNode*> children;

        GameNode(const char src[9], Turn t) : toMove(t), score(0), children()
        {
            for (int i = 0; i < 9; ++i)
            {
                board[i] = src[i];
            }
        }

        ~GameNode()
        {
            for (GameNode* c : children)
            {
                delete c;
            }
        }
    };

    char g_board[9] = {'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b'};
    GameState g_state = GameState::Playing;

    static bool lineWin(const char b[9], char sym, int a, int x, int y)
    {
        return b[a] == sym && b[x] == sym && b[y] == sym;
    }

    static bool terminalScore(const char b[9], int& scoreOut)
    {
        const int lines[8][3] = {
            {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6},
            {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

        for (int k = 0; k < 2; ++k)
        {
            const char sym = (k == 0) ? 'x' : 'o';
            const int s = (sym == 'o') ? 1 : -1;
            for (int L = 0; L < 8; ++L)
            {
                if (lineWin(b, sym, lines[L][0], lines[L][1], lines[L][2]))
                {
                    scoreOut = s;
                    return true;
                }
            }
        }

        for (int i = 0; i < 9; ++i)
        {
            if (b[i] == 'b')
            {
                return false;
            }
        }
        scoreOut = 0;
        return true;
    }

    static int evaluateNode(GameNode* node)
    {
        int leaf = 0;
        if (terminalScore(node->board, leaf))
        {
            node->score = leaf;
            return leaf;
        }

        const char sym = (node->toMove == Turn::Computer) ? 'o' : 'x';
        const Turn next = (node->toMove == Turn::Computer) ? Turn::User : Turn::Computer;

        for (int i = 0; i < 9; ++i)
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
                const int v = evaluateNode(c);
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
                const int v = evaluateNode(c);
                if (v < worst)
                {
                    worst = v;
                }
            }
            node->score = worst;
        }

        return node->score;
    }

    static int diffMoveIndex(const char a[9], const char b[9])
    {
        int idx = -1;
        for (int i = 0; i < 9; ++i)
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

    static int computerChooseMove(const char b[9])
    {
        GameNode* root = new GameNode(b, Turn::Computer);
        evaluateNode(root);

        int bestRating = -2;
        std::vector<int> ties;

        for (GameNode* child : root->children)
        {
            const int mv = diffMoveIndex(b, child->board);
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

        const int n = static_cast<int>(ties.size());
        const int pick = (n <= 0) ? 0 : ties[std::rand() % n];

        delete root;
        return pick;
    }

    static void syncStateFromBoard()
    {
        int score = 0;
        if (!terminalScore(g_board, score))
        {
            g_state = GameState::Playing;
            return;
        }

        if (score == -1)
        {
            g_state = GameState::XWon;
        }
        else if (score == 1)
        {
            g_state = GameState::OWon;
        }
        else
        {
            g_state = GameState::Draw;
        }
    }

    static void maybeDoComputerMove()
    {
        if (g_state != GameState::Playing)
        {
            return;
        }
        const int mv = computerChooseMove(g_board);
        if (mv >= 0 && mv < 9 && g_board[mv] == 'b')
        {
            g_board[mv] = 'o';
            syncStateFromBoard();
        }
    }

    void resetGame()
    {
        for (int i = 0; i < 9; ++i)
        {
            g_board[i] = 'b';
        }
        g_state = GameState::Playing;
    }

    std::wstring statusText()
    {
        switch (g_state)
        {
        case GameState::XWon:
            return L"You win!";
        case GameState::OWon:
            return L"Computer wins.";
        case GameState::Draw:
            return L"Draw.";
        case GameState::Playing:
        default:
            return L"Your turn (X)";
        }
    }

    void drawCenteredText(HDC hdc, const RECT& rect, const wchar_t* text, int fontHeight, int weight)
    {
        HFONT font = CreateFontW(
            fontHeight, 0, 0, 0, weight, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, font));

        SetBkMode(hdc, TRANSPARENT);
        DrawTextW(hdc, text, -1, const_cast<RECT*>(&rect), DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdc, oldFont);
        DeleteObject(font);
    }

    void paintBoard(HDC hdc)
    {
        HPEN gridPen = CreatePen(PS_SOLID, 3, RGB(30, 30, 30));
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, gridPen));

        Rectangle(hdc, kBoardLeft, kBoardTop, kBoardLeft + kBoardSize, kBoardTop + kBoardSize);

        for (int i = 1; i <= 2; ++i)
        {
            MoveToEx(hdc, kBoardLeft + i * kCellSize, kBoardTop, nullptr);
            LineTo(hdc, kBoardLeft + i * kCellSize, kBoardTop + kBoardSize);
            MoveToEx(hdc, kBoardLeft, kBoardTop + i * kCellSize, nullptr);
            LineTo(hdc, kBoardLeft + kBoardSize, kBoardTop + i * kCellSize);
        }

        SelectObject(hdc, oldPen);
        DeleteObject(gridPen);

        for (int i = 0; i < 9; ++i)
        {
            if (g_board[i] == 'b')
            {
                continue;
            }

            const int r = i / 3;
            const int c = i % 3;
            RECT cellRect{
                kBoardLeft + c * kCellSize,
                kBoardTop + r * kCellSize,
                kBoardLeft + (c + 1) * kCellSize,
                kBoardTop + (r + 1) * kCellSize};

            const wchar_t ch = (g_board[i] == 'x') ? L'X' : L'O';
            wchar_t symbol[2] = {ch, L'\0'};
            drawCenteredText(hdc, cellRect, symbol, 62, FW_BOLD);
        }
    }

    void paintStatus(HDC hdc)
    {
        RECT statusRect{20, kStatusTop, kWindowWidth - 20, kStatusTop + 32};
        const std::wstring text = statusText();
        drawCenteredText(hdc, statusRect, text.c_str(), 28, FW_SEMIBOLD);
    }

    void paintResetButton(HDC hdc)
    {
        RECT buttonRect{
            kButtonLeft,
            kButtonTop,
            kButtonLeft + kButtonWidth,
            kButtonTop + kButtonHeight};

        HBRUSH brush = CreateSolidBrush(RGB(70, 130, 180));
        FillRect(hdc, &buttonRect, brush);
        DeleteObject(brush);

        FrameRect(hdc, &buttonRect, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        drawCenteredText(hdc, buttonRect, L"New Game", 20, FW_MEDIUM);
    }

    bool pointInResetButton(int x, int y)
    {
        return x >= kButtonLeft && x <= (kButtonLeft + kButtonWidth) &&
               y >= kButtonTop && y <= (kButtonTop + kButtonHeight);
    }

    bool pointInBoard(int x, int y)
    {
        return x >= kBoardLeft && x < (kBoardLeft + kBoardSize) &&
               y >= kBoardTop && y < (kBoardTop + kBoardSize);
    }

    void handleBoardClick(int x, int y)
    {
        if (g_state != GameState::Playing || !pointInBoard(x, y))
        {
            return;
        }

        const int col = (x - kBoardLeft) / kCellSize;
        const int row = (y - kBoardTop) / kCellSize;
        const int idx = row * 3 + col;

        if (row < 0 || row >= 3 || col < 0 || col >= 3 || g_board[idx] != 'b')
        {
            return;
        }

        g_board[idx] = 'x';
        syncStateFromBoard();
        maybeDoComputerMove();
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        resetGame();
        return 0;

    case WM_LBUTTONDOWN:
    {
        const int x = GET_X_LPARAM(lParam);
        const int y = GET_Y_LPARAM(lParam);

        if (pointInResetButton(x, y))
        {
            resetGame();
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }

        handleBoardClick(x, y);
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        HBRUSH bgBrush = CreateSolidBrush(RGB(245, 245, 245));
        FillRect(hdc, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);

        paintBoard(hdc);
        paintStatus(hdc);
        paintResetButton(hdc);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    const wchar_t kClassName[] = L"TicTacToeWindowClass";

    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));

    if (!RegisterClassW(&wc))
    {
        MessageBoxW(nullptr, L"Failed to register window class.", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExW(
        0,
        kClassName,
        L"Tic-Tac-Toe",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        kWindowWidth, kWindowHeight,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hwnd)
    {
        MessageBoxW(nullptr, L"Failed to create window.", L"Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}
