#pragma once

#include <array>
#include <cstdint>
#include <vector>

// ─────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────
static constexpr int BOARD_SIZE  = 19;
static constexpr int BOARD_CELLS = BOARD_SIZE * BOARD_SIZE; // 361

// ─────────────────────────────────────────────
//  Cell state
// ─────────────────────────────────────────────
enum class Cell : int8_t
{
    EMPTY = 0,
    BLACK = 1,   // Player 1
    WHITE = 2    // Player 2
};

// Return the opponent of a given player
inline Cell opponent(Cell player)
{
    return (player == Cell::BLACK) ? Cell::WHITE : Cell::BLACK;
}

// ─────────────────────────────────────────────
//  Move record (for undo history)
// ─────────────────────────────────────────────
struct Move
{
    int  pos;                        // Index in [0, 361)
    Cell player;
    std::vector<int> captured;       // Positions of stones removed by this move
};

// ─────────────────────────────────────────────
//  Board
// ─────────────────────────────────────────────
class Board
{
public:
    Board();

    // ── Coordinate helpers ──────────────────
    static int  index(int row, int col);        // (row, col) → flat index
    static int  row(int idx);                   // flat index → row
    static int  col(int idx);                   // flat index → col
    static bool isValid(int row, int col);      // in-bounds check
    static bool isValidIdx(int idx);

    // ── Cell access ─────────────────────────
    Cell  get(int idx) const;
    Cell  get(int row, int col) const;
    void  set(int idx, Cell value);
    void  set(int row, int col, Cell value);
    bool  isEmpty(int idx) const;

    // ── Move API (used by both game engine and AI) ──
    void applyMove(int pos, Cell player, const std::vector<int>& captured = {});
    void undoMove();

    // ── Capture counters ────────────────────
    int getCaptureCount(Cell player) const;
    void addCaptures(Cell player, int count);

    // ── Utility ─────────────────────────────
    void reset();
    bool isFull() const;
    const std::vector<Move>& history() const;
    int  moveCount() const;

    // ── Debug ───────────────────────────────
    void print() const;

private:
    std::array<Cell, BOARD_CELLS> m_cells;
    std::vector<Move>             m_history;
    int                           m_captures[2]; // [0] = BLACK, [1] = WHITE
};
