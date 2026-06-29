#include "Board.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>

namespace
{
    constexpr int DIRS[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};

    int cellCode(const Board& board, int r, int c, Cell player)
    {
        if (!Board::isValid(r, c))
            return 3;
        Cell v = board.get(r, c);
        if (v == player)
            return 1;
        if (v == Cell::EMPTY)
            return 0;
        return 2;
    }
}

std::array<std::array<uint64_t, 3>, BOARD_CELLS> Board::s_zobrist{};
bool Board::s_zobristInit = false;

void Board::initZobrist()
{
    if (s_zobristInit)
        return;
    uint64_t seed = 0x9E3779B97F4A7C15ULL;
    for (int i = 0; i < BOARD_CELLS; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            seed ^= seed >> 12;
            seed ^= seed << 25;
            seed ^= seed >> 27;
            s_zobrist[i][j] = seed * 0x2545F4914F6CDD1DULL;
        }
    }
    s_zobristInit = true;
}

Board::Board()
{
    initZobrist();
    reset();
}

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

bool Board::isEmptyCell(int pos) const
{
    return isValidIdx(pos) && isEmpty(pos);
}

void Board::updateHash(int pos, Cell value)
{
    m_hash ^= s_zobrist[pos][static_cast<int>(value)];
}

uint64_t Board::hash() const
{
    return m_hash;
}

void Board::reset()
{
    m_cells.fill(Cell::EMPTY);
    m_history.clear();
    m_captures[0] = 0;
    m_captures[1] = 0;
    m_hash = 0;
}

std::vector<int> Board::computeCaptures(int pos, Cell player) const
{
    std::vector<int> captured;
    Cell opp = opponent(player);
    int  r   = row(pos);
    int  c   = col(pos);

    for (int d = 0; d < 4; ++d)
    {
        int dr = DIRS[d][0];
        int dc = DIRS[d][1];

        auto tryCapture = [&](int o1r, int o1c, int o2r, int o2c, int fr, int fc) {
            if (!isValid(o1r, o1c) || !isValid(o2r, o2c) || !isValid(fr, fc))
                return;
            if (get(o1r, o1c) != opp || get(o2r, o2c) != opp)
                return;
            if (get(fr, fc) != player)
                return;
            int i1 = index(o1r, o1c);
            int i2 = index(o2r, o2c);
            if (std::find(captured.begin(), captured.end(), i1) == captured.end())
                captured.push_back(i1);
            if (std::find(captured.begin(), captured.end(), i2) == captured.end())
                captured.push_back(i2);
        };

        tryCapture(r + dr, c + dc, r + 2 * dr, c + 2 * dc, r + 3 * dr, c + 3 * dc);
        tryCapture(r - dr, c - dc, r - 2 * dr, c - 2 * dc, r - 3 * dr, c - 3 * dc);
        tryCapture(r + dr, c + dc, r + 2 * dr, c + 2 * dc, r - dr, c - dc);
        tryCapture(r - dr, c - dc, r - 2 * dr, c - 2 * dc, r + dr, c + dc);
    }

    return captured;
}

void Board::extractLine(int pos, int dir, Cell player, int line[9]) const
{
    int r  = row(pos);
    int c  = col(pos);
    int dr = DIRS[dir][0];
    int dc = DIRS[dir][1];

    for (int i = -4; i <= 4; ++i)
        line[i + 4] = cellCode(*this, r + i * dr, c + i * dc, player);
    line[4] = 1;
}

bool Board::wouldCreateOpenFour(const int line[9]) const
{
    for (int start = 0; start <= 5; ++start)
    {
        if (line[start] != 1 || line[start + 1] != 1 ||
            line[start + 2] != 1 || line[start + 3] != 1)
            continue;

        bool leftOpen  = (start > 0 && line[start - 1] == 0);
        bool rightOpen = (start + 4 < 9 && line[start + 4] == 0);
        if (leftOpen && rightOpen)
            return true;
    }

    for (int start = 0; start <= 4; ++start)
    {
        if (line[start] == 0 && line[start + 1] == 1 && line[start + 2] == 1 &&
            line[start + 3] == 1 && line[start + 4] == 1 && start + 5 < 9 &&
            line[start + 5] == 0)
            return true;

        if (start + 5 < 9 && line[start] == 0 && line[start + 1] == 1 &&
            line[start + 2] == 1 && line[start + 3] == 0 && line[start + 4] == 1 &&
            line[start + 5] == 0)
            return true;

        if (start + 5 < 9 && line[start] == 0 && line[start + 1] == 1 &&
            line[start + 2] == 0 && line[start + 3] == 1 && line[start + 4] == 1 &&
            line[start + 5] == 0)
            return true;
    }

    return false;
}

bool Board::isFreeThreeInDirection(int pos, int dir, Cell player) const
{
    int line[9];
    extractLine(pos, dir, player, line);

    int stoneCount = 0;
    for (int i = 0; i < 9; ++i)
        if (line[i] == 1)
            ++stoneCount;
    if (stoneCount < 3)
        return false;

    for (int i = 0; i < 9; ++i)
    {
        if (line[i] != 0)
            continue;
        int trial[9];
        for (int j = 0; j < 9; ++j)
            trial[j] = line[j];
        trial[i] = 1;
        if (wouldCreateOpenFour(trial))
            return true;
    }
    return false;
}

int Board::countFreeThrees(int pos, Cell player) const
{
    int count = 0;
    for (int d = 0; d < 4; ++d)
    {
        if (isFreeThreeInDirection(pos, d, player) && ++count >= 2)
            return count;
    }
    return count;
}

bool Board::isLegal(int pos, Cell player) const
{
    if (!isValidIdx(pos) || !isEmpty(pos))
        return false;

    std::vector<int> captured = computeCaptures(pos, player);
    if (countFreeThrees(pos, player) >= 2 && captured.empty())
        return false;

    return true;
}

std::vector<int> Board::applyMove(int pos, Cell player)
{
    std::vector<int> captured = computeCaptures(pos, player);

    updateHash(pos, player);
    m_cells[pos] = player;

    for (int cap : captured)
    {
        updateHash(cap, Cell::EMPTY);
        m_cells[cap] = Cell::EMPTY;
    }

    if (!captured.empty())
    {
        int playerIdx = (player == Cell::BLACK) ? 0 : 1;
        m_captures[playerIdx] += static_cast<int>(captured.size());
    }

    m_history.push_back({pos, player, captured});
    return captured;
}

void Board::undoMove()
{
    if (m_history.empty())
        return;

    const Move& last = m_history.back();

    updateHash(last.pos, Cell::EMPTY);
    m_cells[last.pos] = Cell::EMPTY;

    for (int cap : last.captured)
    {
        updateHash(cap, opponent(last.player));
        m_cells[cap] = opponent(last.player);
    }

    if (!last.captured.empty())
    {
        int playerIdx = (last.player == Cell::BLACK) ? 0 : 1;
        m_captures[playerIdx] -= static_cast<int>(last.captured.size());
    }

    m_history.pop_back();
}

int Board::countAlignment(int pos, Cell player) const
{
    int r = row(pos);
    int c = col(pos);
    int best = 0;

    for (int d = 0; d < 4; ++d)
    {
        int dr = DIRS[d][0];
        int dc = DIRS[d][1];
        int count = 1;

        for (int i = 1; i < BOARD_SIZE; ++i)
        {
            int nr = r + i * dr;
            int nc = c + i * dc;
            if (!isValid(nr, nc) || get(nr, nc) != player)
                break;
            ++count;
        }
        for (int i = 1; i < BOARD_SIZE; ++i)
        {
            int nr = r - i * dr;
            int nc = c - i * dc;
            if (!isValid(nr, nc) || get(nr, nc) != player)
                break;
            ++count;
        }
        best = std::max(best, count);
    }
    return best;
}

bool Board::hasAlignmentWin(int pos, Cell player) const
{
    return countAlignment(pos, player) >= 5;
}

bool Board::canOpponentBreakAlignment(Cell player, int lastPos) const
{
    Cell opp = opponent(player);
  std::vector<int> lineStones;

    int r = row(lastPos);
    int c = col(lastPos);

    for (int d = 0; d < 4; ++d)
    {
        int dr = DIRS[d][0];
        int dc = DIRS[d][1];
        std::vector<int> run;

        for (int i = -4; i <= 4; ++i)
        {
            int nr = r + i * dr;
            int nc = c + i * dc;
            if (!isValid(nr, nc) || get(nr, nc) != player)
                continue;
            run.push_back(index(nr, nc));
        }

        if (static_cast<int>(run.size()) >= 5)
        {
            for (int idx : run)
            {
                if (std::find(lineStones.begin(), lineStones.end(), idx) == lineStones.end())
                    lineStones.push_back(idx);
            }
        }
    }

    if (lineStones.empty())
        return false;

    std::vector<int> candidates = getCandidateMoves(3);
    for (int pos : candidates)
    {
        if (!isEmpty(pos))
            continue;
        std::vector<int> caps = computeCaptures(pos, opp);
        if (caps.empty())
            continue;
        for (int cap : caps)
        {
            if (std::find(lineStones.begin(), lineStones.end(), cap) != lineStones.end())
                return true;
        }
    }
    return false;
}

WinReason Board::checkWin(Cell player, int lastPos) const
{
    if (getCaptureCount(player) >= 10)
        return WinReason::CAPTURE;

    if (hasAlignmentWin(lastPos, player))
    {
        if (!canOpponentBreakAlignment(player, lastPos))
            return WinReason::ALIGNMENT;
    }

    return WinReason::NONE;
}

WinReason Board::checkWinFast(Cell player, int lastPos) const
{
    if (getCaptureCount(player) >= 10)
        return WinReason::CAPTURE;
    if (hasAlignmentWin(lastPos, player))
        return WinReason::ALIGNMENT;
    return WinReason::NONE;
}

int Board::quickMoveScore(int pos, Cell player) const
{
    int  score = 0;
    auto caps  = computeCaptures(pos, player);
    score += static_cast<int>(caps.size()) * 1000;

    int r = row(pos);
    int c = col(pos);
    for (int dr = -1; dr <= 1; ++dr)
    {
        for (int dc = -1; dc <= 1; ++dc)
        {
            if (dr == 0 && dc == 0)
                continue;
            int nr = r + dr;
            int nc = c + dc;
            if (!isValid(nr, nc))
                continue;
            Cell v = get(nr, nc);
            if (v == player)
                score += 50;
            else if (v == opponent(player))
                score += 25;
        }
    }

    score += 18 - (std::abs(r - BOARD_SIZE / 2) + std::abs(c - BOARD_SIZE / 2));
    return score;
}

int Board::getCaptureCount(Cell player) const
{
    return m_captures[(player == Cell::BLACK) ? 0 : 1];
}

std::vector<int> Board::getCandidateMoves(int radius) const
{
    std::vector<int> candidates;
    bool             hasStone = false;

    for (int i = 0; i < BOARD_CELLS; ++i)
    {
        if (m_cells[i] != Cell::EMPTY)
        {
            hasStone = true;
            break;
        }
    }

    if (!hasStone)
    {
        candidates.push_back(index(BOARD_SIZE / 2, BOARD_SIZE / 2));
        return candidates;
    }

    std::array<bool, BOARD_CELLS> marked{};
    for (int i = 0; i < BOARD_CELLS; ++i)
    {
        if (m_cells[i] == Cell::EMPTY)
            continue;
        int r = row(i);
        int c = col(i);
        for (int dr = -radius; dr <= radius; ++dr)
        {
            for (int dc = -radius; dc <= radius; ++dc)
            {
                int nr = r + dr;
                int nc = c + dc;
                if (!isValid(nr, nc))
                    continue;
                int idx = index(nr, nc);
                if (m_cells[idx] == Cell::EMPTY && !marked[idx])
                {
                    marked[idx] = true;
                    candidates.push_back(idx);
                }
            }
        }
    }

    if (candidates.empty())
    {
        for (int i = 0; i < BOARD_CELLS; ++i)
            if (m_cells[i] == Cell::EMPTY)
                candidates.push_back(i);
    }

    return candidates;
}

bool Board::isFull() const
{
    for (const auto& cell : m_cells)
        if (cell == Cell::EMPTY)
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

void Board::print() const
{
    const char* symbols[] = {" .", " B", " W"};

    std::cout << "   ";
    for (int colIdx = 0; colIdx < BOARD_SIZE; ++colIdx)
        std::cout << std::setw(2) << colIdx;
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
