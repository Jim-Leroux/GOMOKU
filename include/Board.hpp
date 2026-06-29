#pragma once

#include <array>
#include <cstdint>
#include <vector>

static constexpr int BOARD_SIZE  = 19;
static constexpr int BOARD_CELLS = BOARD_SIZE * BOARD_SIZE;

enum class Cell : int8_t
{
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

enum class WinReason : int8_t
{
    NONE = 0,
    ALIGNMENT,
    CAPTURE
};

inline Cell opponent(Cell player)
{
    return (player == Cell::BLACK) ? Cell::WHITE : Cell::BLACK;
}

struct Move
{
    int              pos;
    Cell             player;
    std::vector<int> captured;
};

class Board
{
public:
    Board();
    Board(const Board& other)            = default;
    Board& operator=(const Board& other)   = default;

    static int  index(int row, int col);
    static int  row(int idx);
    static int  col(int idx);
    static bool isValid(int row, int col);
    static bool isValidIdx(int idx);

    Cell  get(int idx) const;
    Cell  get(int row, int col) const;
    void  set(int idx, Cell value);
    void  set(int row, int col, Cell value);
    bool  isEmpty(int idx) const;

  // Rules API — used by GameEngine and AI search
    bool isLegal(int pos, Cell player) const;
    bool isEmptyCell(int pos) const;
    std::vector<int> applyMove(int pos, Cell player);
    void undoMove();

    WinReason checkWin(Cell player, int lastPos) const;
    WinReason checkWinFast(Cell player, int lastPos) const;
    int  getCaptureCount(Cell player) const;
    int  quickMoveScore(int pos, Cell player) const;

    std::vector<int> getCandidateMoves(int radius = 2) const;

    uint64_t hash() const;

    void reset();
    bool isFull() const;
    const std::vector<Move>& history() const;
    int  moveCount() const;

    void print() const;

private:
    std::array<Cell, BOARD_CELLS> m_cells;
    std::vector<Move>             m_history;
    int                           m_captures[2];
    uint64_t                      m_hash;

    static std::array<std::array<uint64_t, 3>, BOARD_CELLS> s_zobrist;
    static bool s_zobristInit;

    static void initZobrist();

    void updateHash(int pos, Cell value);

    std::vector<int> computeCaptures(int pos, Cell player) const;
    int  countAlignment(int pos, Cell player) const;
    bool hasAlignmentWin(int pos, Cell player) const;
    bool canOpponentBreakAlignment(Cell player, int lastPos) const;

    int  countFreeThrees(int pos, Cell player) const;
    bool isFreeThreeInDirection(int pos, int dir, Cell player) const;
    bool wouldCreateOpenFour(const int line[9]) const;
    void extractLine(int pos, int dir, Cell player, int line[9]) const;
};
