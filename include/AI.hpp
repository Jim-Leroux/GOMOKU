#pragma once

#include "GameEngine.hpp"

#include <chrono>
#include <cstddef>

struct AIStats
{
    int depthReached = 0;
    long elapsedMs = 0;
};

class AI
{
public:
    explicit AI(Cell aiPlayer) : m_aiPlayer(aiPlayer), m_tt(TT_SIZE) {}
    Cell player() const { return m_aiPlayer; }
    int evaluate(const Board& board) const;

    int findBestMove(GameEngine& engine, int depth);
    int findBestMoveTimed(GameEngine& engine, std::chrono::milliseconds budget);

    const AIStats& lastStats() const { return m_lastStats; }
    long long lastThinkingTimeMs() const { return m_lastThinkingTimeMs; }
    int lastSearchDepth() const { return m_lastSearchDepth; }

    using Clock = std::chrono::steady_clock; // Measures time independently of the system clock

private:
    Cell m_aiPlayer;
    long long m_lastThinkingTimeMs = 0;
    int m_lastSearchDepth = 0;
    AIStats m_lastStats;

    static constexpr int WIN_SCORE = 100000000;
    static constexpr int MAX_CANDIDATES = 8;
    static constexpr int MAX_DEPTH = 12;
    
    enum : uint8_t { TT_EXACT, TT_LOWER, TT_UPPER };
    struct TTEntry
    {
        uint64_t key = 0;
        int score = 0;
        int depth = -1;
        int bestMove = -1;
        uint8_t flag = TT_EXACT;
    };
    static constexpr size_t TT_SIZE = size_t(1) << 20; // 2^20 = 1 048 576 slots

    std::vector<TTEntry> m_tt; // Transposition table
    uint64_t m_hash = 0; // Hash of the current board position
    
    std::vector<int> nearbyEmptyCells(GameEngine& engine) const;
    int quickMoveScore(const Board& board, int move, Cell player) const;

    std::vector<int> getBestMoves(GameEngine& engine, int priorityMove = -1);
    int minMax(GameEngine& engine, int depth,
        int alpha, int beta, bool maximizing,
        Clock::time_point deadline = Clock::time_point::max());

    void initSearch(GameEngine& engine);
    void playHashed(GameEngine& engine, int move);
    void undoHashed(GameEngine& engine);
    uint64_t positionKey(GameEngine& engine) const;
};
