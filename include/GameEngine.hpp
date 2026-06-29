#pragma once

#include "Board.hpp"

#include <chrono>
#include <cstdint>
#include <vector>

enum class GameMode
{
    HUMAN_VS_AI,
    HOTSEAT
};

enum class TTFlag : int8_t
{
    EXACT,
    LOWER,
    UPPER
};

struct TTEntry
{
    uint64_t hash  = 0;
    int      depth = 0;
    int      score = 0;
    TTFlag   flag  = TTFlag::EXACT;
    int      bestMove = -1;
};

class GameEngine
{
public:
    GameEngine();

    const Board& getBoard() const;

    void setMode(GameMode mode);
    GameMode getMode() const;

    bool isLegalMove(int pos) const;
    void playMove(int pos);
    void undoMove();

    bool     isTerminal() const;
    Cell     getWinner() const;
    WinReason getWinReason() const;
    Cell     getCurrentPlayer() const;
    int      getMoveCount() const;
    int      getCaptureCount(Cell player) const;

    int  getLastMove() const;
    int  getSuggestedMove() const;
    long getLastAiTimeMs() const;
    int  getLastSearchDepth() const;

    void playAiMove();
    void updateSuggestion();

    void reset();

private:
    static constexpr int    MAX_DEPTH        = 12;
    static constexpr int    MAX_SEARCH_MOVES = 8;
    static constexpr int    TT_SIZE          = 1 << 20;
    static constexpr int    INF_SCORE        = 10000000;
    static constexpr double TIME_LIMIT_S     = 0.5;

    Board    m_board;
    Cell     m_currentPlayer;
    Cell     m_winner;
    WinReason m_winReason;
    bool     m_terminal;
    GameMode m_mode;

    int  m_lastMove;
    int  m_suggestedMove;
    long m_lastAiTimeMs;
    int  m_lastSearchDepth;

    std::vector<TTEntry> m_tt;

    struct SearchStats
    {
        int nodes = 0;
        std::chrono::steady_clock::time_point deadline;
    };

    int  negamax(int depth, int alpha, int beta, Cell rootPlayer, SearchStats& stats);
    int  collectSearchMoves(Cell player, int* out, bool legalOnly) const;
    int  evaluate(Cell rootPlayer) const;
    int  evaluatePositional(Cell rootPlayer) const;
    int  terminalScore(Cell mover, WinReason reason, int depth, Cell rootPlayer) const;

    void storeTT(uint64_t hash, int depth, int score, TTFlag flag, int bestMove);
    bool probeTT(uint64_t hash, int depth, int alpha, int beta, int& score, int& bestMove) const;

    int  chooseAiMove();
};
