#include "GameEngine.hpp"
#include <stdexcept>

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

    // TODO (tâche 2.1) : détecter alignement de 5+
    // TODO (tâche 2.2) : victoire par captures (>= 10)
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
    (void)pos;
    (void)player;
    return {}; // TODO tâche 2.2
}

bool GameEngine::checkAlignment(int pos, Cell player) const
{
    (void)pos;
    (void)player;
    return false; // TODO tâche 2.1
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
