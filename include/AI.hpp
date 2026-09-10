#pragma once

#include "GameEngine.hpp"

class AI
{
public:
    explicit AI(Cell aiPlayer) : m_aiPlayer(aiPlayer) {}
    Cell player() const { return m_aiPlayer; }
    int evaluate(const Board& board) const;

private:
    Cell m_aiPlayer;
};
