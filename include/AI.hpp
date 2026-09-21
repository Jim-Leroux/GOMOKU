#pragma once

#include "GameEngine.hpp"

class AI
{
public:
    explicit AI(Cell aiPlayer) : m_aiPlayer(aiPlayer) {}
    Cell player() const { return m_aiPlayer; }
    int evaluate(const Board& board) const;
    int findBestMove(GameEngine& engine, int depth);
    long long lastThinkingTimeMs() const { return m_lastThinkingTimeMs; }
    int lastSearchDepth() const { return m_lastSearchDepth; }

private:
    Cell m_aiPlayer;
    long long m_lastThinkingTimeMs = 0;
    int m_lastSearchDepth = 0;
    static constexpr int WIN_SCORE = 100000000;
    static constexpr int MAX_CANDIDATES = 8;
    std::vector<int> nearbyEmptyCells(GameEngine& engine) const;
    int quickMoveScore(const Board& board, int move, Cell player) const;
    std::vector<int> getBestMoves(GameEngine& engine);
    int minMax(GameEngine& engine, int depth, int alpha, int beta, bool maximizing);
};
