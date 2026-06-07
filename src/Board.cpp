#include "Board.hpp"

#include <stdexcept>
#include <iostream>
#include <iomanip>

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
Board::Board()
{
    reset();
}

// ─────────────────────────────────────────────
//  Coordinate helpers
// ─────────────────────────────────────────────
int Board::index(int row, int col)
{
    return row * BOARD_SIZE + col;
}

int Board::row(int idx)
{
    return idx / BOARD_SIZE;
}

int Board::col(int idx)
{
    return idx % BOARD_SIZE;
}

bool Board::isValid(int row, int col)
{
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

bool Board::isValidIdx(int idx)
{
    return idx >= 0 && idx < BOARD_CELLS;
}

// ─────────────────────────────────────────────
//  Cell access
// ─────────────────────────────────────────────
Cell Board::get(int idx) const
{
    return m_cells[idx];
}

Cell Board::get(int r, int c) const
{
    return m_cells[index(r, c)];
}

void Board::set(int idx, Cell value)
{
    m_cells[idx] = value;
}

void Board::set(int r, int c, Cell value)
{
    m_cells[index(r, c)] = value;
}

bool Board::isEmpty(int idx) const
{
    return m_cells[idx] == Cell::EMPTY;
}

// ─────────────────────────────────────────────
//  Move API
// ─────────────────────────────────────────────
void Board::applyMove(int pos, Cell player, const std::vector<int>& captured)
{
    // Place the stone
    m_cells[pos] = player;

    // Remove captured stones
    for (int cap : captured)
        m_cells[cap] = Cell::EMPTY;

    // Update capture counter (pairs = captured.size() / 2)
    if (!captured.empty())
    {
        int playerIdx = (player == Cell::BLACK) ? 0 : 1;
        m_captures[playerIdx] += static_cast<int>(captured.size());
    }

    // Push to history
    m_history.push_back({pos, player, captured});
}

void Board::undoMove()
{
    if (m_history.empty())
        return;

    const Move& last = m_history.back();

    // Remove placed stone
    m_cells[last.pos] = Cell::EMPTY;

    // Restore captured stones
    for (int cap : last.captured)
        m_cells[cap] = opponent(last.player);

    // Revert capture counter
    if (!last.captured.empty())
    {
        int playerIdx = (last.player == Cell::BLACK) ? 0 : 1;
        m_captures[playerIdx] -= static_cast<int>(last.captured.size());
    }

    m_history.pop_back();
}

// ─────────────────────────────────────────────
//  Capture counters
// ─────────────────────────────────────────────
int Board::getCaptureCount(Cell player) const
{
    return m_captures[(player == Cell::BLACK) ? 0 : 1];
}

void Board::addCaptures(Cell player, int count)
{
    m_captures[(player == Cell::BLACK) ? 0 : 1] += count;
}

// ─────────────────────────────────────────────
//  Utility
// ─────────────────────────────────────────────
void Board::reset()
{
    m_cells.fill(Cell::EMPTY);
    m_history.clear();
    m_captures[0] = 0;
    m_captures[1] = 0;
}

bool Board::isFull() const
{
    for (const auto& c : m_cells)
        if (c == Cell::EMPTY)
            return false;
    return true;
}

const std::vector<Move>& Board::history() const
{
    return m_history;
}

int Board::moveCount() const
{
    return static_cast<int>(m_history.size());
}

// ─────────────────────────────────────────────
//  Debug print
// ─────────────────────────────────────────────
void Board::print() const
{
    const char* symbols[] = {" .", " B", " W"};

    std::cout << "   ";
    for (int c = 0; c < BOARD_SIZE; ++c)
        std::cout << std::setw(2) << c;
    std::cout << "\n";

    for (int r = 0; r < BOARD_SIZE; ++r)
    {
        std::cout << std::setw(2) << r << " ";
        for (int c = 0; c < BOARD_SIZE; ++c)
            std::cout << symbols[static_cast<int>(get(r, c))];
        std::cout << "\n";
    }

    std::cout << "BLACK captures: " << m_captures[0]
              << " | WHITE captures: " << m_captures[1] << "\n";
}
