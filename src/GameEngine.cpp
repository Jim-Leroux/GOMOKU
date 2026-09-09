#include "GameEngine.hpp"
#include <stdexcept>

namespace
{
    constexpr int AXES[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };
    constexpr int DIRS[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},          { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    };

    Cell cellIfPlayed(const Board& board, int index, int pos, Cell player)
    {
        if (index == pos)
            return player;
        if (!Board::isValidIdx(index))
            return opponent(player);
        return board.get(index);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
GameEngine::GameEngine()
    : m_currentPlayer(Cell::BLACK)
    , m_winner(Cell::EMPTY)
    , m_terminal(false)
{
}

// ─────────────────────────────────────────────────────────────────────────────
//  Board access
// ─────────────────────────────────────────────────────────────────────────────
const Board& GameEngine::getBoard() const
{
    return m_board;
}

// ─────────────────────────────────────────────────────────────────────────────
//  isLegalMove
//  Vérifie qu'un coup est jouable pour le joueur courant.
//  Les règles seront ajoutées progressivement dans les tâches suivantes.
// ─────────────────────────────────────────────────────────────────────────────
bool GameEngine::isLegalMove(int pos) const
{
    if (!Board::isValidIdx(pos))
        return false;
    if (!m_board.isEmpty(pos))
        return false;
    if (m_terminal)
        return false;
    if (!computeCaptures(pos, m_currentPlayer).empty())
        return true;
    return !checkDoubleThree(pos, m_currentPlayer);
}

// ─────────────────────────────────────────────────────────────────────────────
//  playMove
//  Joue un coup pour le joueur courant.
// ─────────────────────────────────────────────────────────────────────────────
void GameEngine::playMove(int pos)
{
    if (!isLegalMove(pos))
        throw std::invalid_argument("GameEngine::playMove — coup illégal");

    std::vector<int> captured = computeCaptures(pos, m_currentPlayer);
    m_board.applyMove(pos, m_currentPlayer, captured);
    std::vector<int> line = getAlignmentStones(pos, m_currentPlayer);

    if (!line.empty() && !checkEndgameCapture(m_currentPlayer, line))
    {
        m_winner = m_currentPlayer;
        m_terminal = true;
    }
    
    if (getCaptureCount(m_currentPlayer) >= 10)
    {
        m_winner = m_currentPlayer;
        m_terminal = true;
    }

    m_currentPlayer = opponent(m_currentPlayer);
}

// ─────────────────────────────────────────────────────────────────────────────
//  undoMove
// ─────────────────────────────────────────────────────────────────────────────
void GameEngine::undoMove()
{
    if (m_board.moveCount() == 0)
        return;

    m_currentPlayer = m_board.history().back().player;
    m_board.undoMove();
    m_terminal = false;
    m_winner   = Cell::EMPTY;
}

// ─────────────────────────────────────────────────────────────────────────────
//  État de la partie
// ─────────────────────────────────────────────────────────────────────────────
bool GameEngine::isTerminal() const
{
    return m_terminal || m_board.isFull();
}

Cell GameEngine::getWinner() const
{
    return m_winner;
}

Cell GameEngine::getCurrentPlayer() const
{
    return m_currentPlayer;
}

int GameEngine::getMoveCount() const
{
    return m_board.moveCount();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Captures
// ─────────────────────────────────────────────────────────────────────────────
int GameEngine::getCaptureCount(Cell player) const
{
    return m_board.getCaptureCount(player);
}

// ─────────────────────────────────────────────────────────────────────────────
//  reset
// ─────────────────────────────────────────────────────────────────────────────
void GameEngine::reset()
{
    m_board.reset();
    m_currentPlayer = Cell::BLACK;
    m_winner        = Cell::EMPTY;
    m_terminal      = false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers privés (stubs — implémentés dans les tâches suivantes)
// ─────────────────────────────────────────────────────────────────────────────
std::vector<int> GameEngine::computeCaptures(int pos, Cell player) const
{
    Cell opp = opponent(player);
    std::vector<int> captured;
    int row = Board::row(pos), col = Board::col(pos);
    for (const int (&dir)[2] : DIRS)
    {
        int row1 = row + dir[0], col1 = col + dir[1];
        int row2 = row + dir[0] * 2, col2 = col + dir[1] * 2;
        int row3 = row + dir[0] * 3, col3 = col + dir[1] * 3;
        if (!Board::isValid(row3, col3))
            continue;
        if (m_board.get(row1, col1) == opp
            && m_board.get(row2, col2) == opp
            && m_board.get(row3, col3) == player)
        {
            captured.push_back(Board::index(row1, col1));
            captured.push_back(Board::index(row2, col2));
        }
    }
    return captured;
}

std::vector<int> GameEngine::getAlignmentStones(int pos, Cell player) const
{
    int row = Board::row(pos), col = Board::col(pos);
    std::vector<int> result;
    for (const int (&axis)[2] : AXES)
    {
        int deltaRow = axis[0], deltaCol = axis[1];
        std::vector<int> line = {pos};
        int nextRow = row + deltaRow, nextCol = col + deltaCol;
        while (Board::isValid(nextRow, nextCol) && m_board.get(nextRow, nextCol) == player)
        {
            line.push_back(Board::index(nextRow, nextCol));
            nextRow += deltaRow, nextCol += deltaCol;
        }
        nextRow = row - deltaRow, nextCol = col - deltaCol;
        while (Board::isValid(nextRow, nextCol) && m_board.get(nextRow, nextCol) == player)
        {
            line.push_back(Board::index(nextRow, nextCol));
            nextRow -= deltaRow, nextCol -= deltaCol;
        }
        if (line.size() >= 5)
            result.insert(result.end(), line.begin(), line.end());
    }
    return result;
}

bool GameEngine::checkDoubleThree(int pos, Cell player) const
{
    int freeAxes = 0;
    int row = Board::row(pos), col = Board::col(pos);
    for (const int (&axis)[2] : AXES)
    {
        int deltaRow = axis[0], deltaCol = axis[1];
        for (int offset = -3; offset <= 0; ++offset)
        {
            int selfCount = 0, emptyCount = 0;
            bool blocked = false;
            for (int i = 0; i < 4 && !blocked; ++i)
            {
                int cellRow = row + (offset + i) * deltaRow, cellCol = col + (offset + i) * deltaCol;
                int index = Board::isValid(cellRow, cellCol) ? Board::index(cellRow, cellCol) : -1;
                Cell cell = cellIfPlayed(m_board, index, pos, player);
                if (cell == player)
                    ++selfCount;
                else if (cell == Cell::EMPTY)
                    ++emptyCount;
                else
                    blocked = true;
            }
            if (blocked || selfCount != 3 || emptyCount != 1)
                continue;

            int beforeRow = row + (offset - 1) * deltaRow, beforeCol = col + (offset - 1) * deltaCol;
            int afterRow = row + (offset + 4) * deltaRow, afterCol = col + (offset + 4) * deltaCol;
            int beforeIndex = Board::isValid(beforeRow, beforeCol) ? Board::index(beforeRow, beforeCol) : -1;
            int afterIndex = Board::isValid(afterRow, afterCol) ? Board::index(afterRow, afterCol) : -1;
            if (cellIfPlayed(m_board, beforeIndex, pos, player) == Cell::EMPTY
                && cellIfPlayed(m_board, afterIndex, pos, player) == Cell::EMPTY)
            {
                ++freeAxes;
                break;
            }
        }
    }
    return freeAxes >= 2;
}

bool GameEngine::checkEndgameCapture(Cell player, const std::vector<int>& line) const
{
    Cell opp = opponent(player);
    for (int stone : line)
    {
        int row = Board::row(stone), col = Board::col(stone);
        for (const int (&dir)[2] : DIRS)
        {
            for (int stoneIndex = 0; stoneIndex < 5; ++stoneIndex)
            {
                int offset = -stoneIndex;
                int row2 = row + offset * dir[0], col2 = col + offset * dir[1];
                int row1 = row2 - dir[0],         col1 = col2 - dir[1];
                int row3 = row2 + dir[0],         col3 = col2 + dir[1];
                int row4 = row3 + dir[0],         col4 = col3 + dir[1];
                if (!Board::isValid(row1, col1) || !Board::isValid(row2, col2)
                    || !Board::isValid(row3, col3) || !Board::isValid(row4, col4))
                    continue;
                if (m_board.get(row1, col1) == Cell::EMPTY
                    && m_board.get(row2, col2) == player
                    && m_board.get(row3, col3) == player
                    && m_board.get(row4, col4) == opp)
                    return true;
            }
        }
    }
    return false;
}
