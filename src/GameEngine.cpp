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

    // TODO (tâche 2.3) : interdire le double-three
    // TODO (tâche 2.2) : interdire de jouer dans une capture

    return true;
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

    if (checkAlignment(pos, m_currentPlayer))
    {
        m_winner = m_currentPlayer;
        m_terminal = true;
    }
    
    if (getCaptureCount(m_currentPlayer) >= 10)
    {
        m_winner = m_currentPlayer;
        m_terminal = true;
    }
    // TODO (tâche 2.4) : vérifier Endgame Capture

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

bool GameEngine::checkAlignment(int pos, Cell player) const
{
    int row = Board::row(pos), col = Board::col(pos);
    for (const int (&axis)[2] : AXES)
    {
        int deltaRow = axis[0], deltaCol = axis[1];
        int count = 1;
        int nextRow = row + deltaRow, nextCol = col + deltaCol;
        while (Board::isValid(nextRow, nextCol) && m_board.get(nextRow, nextCol) == player)
        {
            ++count;
            nextRow += deltaRow, nextCol += deltaCol;
        }
        nextRow = row - deltaRow, nextCol = col - deltaCol;
        while (Board::isValid(nextRow, nextCol) && m_board.get(nextRow, nextCol) == player)
        {
            ++count;
            nextRow -= deltaRow, nextCol -= deltaCol;
        }
        if (count >= 5)
            return true;
    }
    return false;
}

bool GameEngine::checkDoubleThree(int pos, Cell player) const
{
    (void)pos;
    (void)player;
    return false; // TODO tâche 2.3
}

bool GameEngine::checkEndgameCapture(Cell player) const
{
    (void)player;
    return false; // TODO tâche 2.4
}
