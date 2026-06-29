#include "GameEngine.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>

GameEngine::GameEngine()
    : m_currentPlayer(Cell::BLACK)
    , m_winner(Cell::EMPTY)
    , m_winReason(WinReason::NONE)
    , m_terminal(false)
    , m_mode(GameMode::HUMAN_VS_AI)
    , m_lastMove(-1)
    , m_suggestedMove(-1)
    , m_lastAiTimeMs(0)
    , m_lastSearchDepth(0)
    , m_tt(TT_SIZE)
{
}

const Board& GameEngine::getBoard() const
{
    return m_board;
}

void GameEngine::setMode(GameMode mode)
{
    m_mode = mode;
}

GameMode GameEngine::getMode() const
{
    return m_mode;
}

bool GameEngine::isLegalMove(int pos) const
{
    if (m_terminal)
        return false;
    return m_board.isLegal(pos, m_currentPlayer);
}

void GameEngine::playMove(int pos)
{
    if (!isLegalMove(pos))
        throw std::invalid_argument("GameEngine::playMove — illegal move");

    Cell player = m_currentPlayer;
    m_board.applyMove(pos, player);
    m_lastMove = pos;

    std::cout << "\n--- Move " << m_board.moveCount() << ": "
              << (player == Cell::BLACK ? "BLACK" : "WHITE")
              << " at (" << Board::row(pos) << ", " << Board::col(pos) << ") ---\n";
    m_board.print();

    WinReason reason = m_board.checkWin(player, pos);
    if (reason != WinReason::NONE)
    {
        m_terminal   = true;
        m_winner     = player;
        m_winReason  = reason;
        std::cout << ">>> Winner: " << (player == Cell::BLACK ? "BLACK" : "WHITE");
        if (reason == WinReason::ALIGNMENT)
            std::cout << " (5 in a row)\n";
        else
            std::cout << " (10 captures)\n";
        return;
    }

    if (m_board.isFull())
    {
        m_terminal = true;
        std::cout << ">>> Draw (board full)\n";
        return;
    }

    m_currentPlayer = opponent(m_currentPlayer);

    if (m_mode == GameMode::HOTSEAT)
        updateSuggestion();
}

void GameEngine::undoMove()
{
    if (m_board.moveCount() == 0)
        return;

    m_currentPlayer = m_board.history().back().player;
    m_board.undoMove();
    m_terminal      = false;
    m_winner        = Cell::EMPTY;
    m_winReason     = WinReason::NONE;
    m_lastMove      = m_board.moveCount() > 0 ? m_board.history().back().pos : -1;
    m_suggestedMove = -1;
}

bool GameEngine::isTerminal() const
{
    return m_terminal || m_board.isFull();
}

Cell GameEngine::getWinner() const
{
    return m_winner;
}

WinReason GameEngine::getWinReason() const
{
    return m_winReason;
}

Cell GameEngine::getCurrentPlayer() const
{
    return m_currentPlayer;
}

int GameEngine::getMoveCount() const
{
    return m_board.moveCount();
}

int GameEngine::getCaptureCount(Cell player) const
{
    return m_board.getCaptureCount(player);
}

int GameEngine::getLastMove() const
{
    return m_lastMove;
}

int GameEngine::getSuggestedMove() const
{
    return m_suggestedMove;
}

long GameEngine::getLastAiTimeMs() const
{
    return m_lastAiTimeMs;
}

int GameEngine::getLastSearchDepth() const
{
    return m_lastSearchDepth;
}

void GameEngine::reset()
{
    m_board.reset();
    m_currentPlayer = Cell::BLACK;
    m_winner        = Cell::EMPTY;
    m_winReason     = WinReason::NONE;
    m_terminal      = false;
    m_lastMove      = -1;
    m_suggestedMove = -1;
    m_lastAiTimeMs  = 0;
    m_lastSearchDepth = 0;
    std::fill(m_tt.begin(), m_tt.end(), TTEntry{});

    if (m_mode == GameMode::HOTSEAT)
        updateSuggestion();
}

int GameEngine::terminalScore(Cell mover, WinReason reason, int depth, Cell rootPlayer) const
{
  (void)reason;
    if (mover == rootPlayer)
        return INF_SCORE - depth;
    return -INF_SCORE + depth;
}

int GameEngine::evaluatePositional(Cell rootPlayer) const
{
    Cell opp  = opponent(rootPlayer);
    int  score = (m_board.getCaptureCount(rootPlayer) - m_board.getCaptureCount(opp)) * 800;

    constexpr int DIRS[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};

    for (int i = 0; i < BOARD_CELLS; ++i)
    {
        Cell who = m_board.get(i);
        if (who == Cell::EMPTY)
            continue;

        int sign = (who == rootPlayer) ? 1 : -1;
        int r    = Board::row(i);
        int c    = Board::col(i);

        for (int d = 0; d < 4; ++d)
        {
            int dr = DIRS[d][0];
            int dc = DIRS[d][1];

            if (Board::isValid(r - dr, c - dc) && m_board.get(r - dr, c - dc) == who)
                continue;

            int count    = 0;
            int openEnds = 0;
            int nr       = r;
            int nc       = c;
            while (Board::isValid(nr, nc) && m_board.get(nr, nc) == who)
            {
                ++count;
                nr += dr;
                nc += dc;
            }
            if (Board::isValid(nr, nc) && m_board.get(nr, nc) == Cell::EMPTY)
                ++openEnds;

            int local = 0;
            if (count >= 5)
                local = 500000;
            else if (count == 4 && openEnds >= 1)
                local = (openEnds == 2) ? 50000 : 8000;
            else if (count == 3 && openEnds >= 1)
                local = (openEnds == 2) ? 3000 : 400;
            else if (count == 2 && openEnds == 2)
                local = 80;
            else if (count == 2 && openEnds == 1)
                local = 20;

            score += sign * local;
        }
    }

    return score;
}

int GameEngine::evaluate(Cell rootPlayer) const
{
    return evaluatePositional(rootPlayer);
}

void GameEngine::storeTT(uint64_t hash, int depth, int score, TTFlag flag, int bestMove)
{
    size_t slot = hash % TT_SIZE;
    TTEntry& e  = m_tt[slot];
    if (e.hash == 0 || depth >= e.depth)
    {
        e.hash     = hash;
        e.depth    = depth;
        e.score    = score;
        e.flag     = flag;
        e.bestMove = bestMove;
    }
}

bool GameEngine::probeTT(uint64_t hash, int depth, int alpha, int beta,
                         int& score, int& bestMove) const
{
    size_t       slot = hash % TT_SIZE;
    const TTEntry& e  = m_tt[slot];
    if (e.hash != hash || e.depth < depth)
        return false;

    bestMove = e.bestMove;
    score    = e.score;

    if (e.flag == TTFlag::EXACT)
        return true;
    if (e.flag == TTFlag::LOWER && score >= beta)
        return true;
    if (e.flag == TTFlag::UPPER && score <= alpha)
        return true;
    return false;
}

int GameEngine::collectSearchMoves(Cell player, int* out, bool legalOnly) const
{
    struct ScoredMove
    {
        int pos;
        int score;
    };

    std::vector<int> raw = m_board.getCandidateMoves(2);
    ScoredMove       scored[64];
    int              count = 0;

    for (int pos : raw)
    {
        if (legalOnly)
        {
            if (!m_board.isLegal(pos, player))
                continue;
        }
        else if (!m_board.isEmptyCell(pos))
        {
            continue;
        }
        if (count >= 64)
            break;
        scored[count++] = {pos, m_board.quickMoveScore(pos, player)};
    }

    if (count == 0)
        return 0;

    int maxMoves = MAX_SEARCH_MOVES;
    if (m_board.moveCount() < 3)
        maxMoves = 3;
    else if (m_board.moveCount() < 8)
        maxMoves = 5;

    int keep = std::min(count, maxMoves);
    std::partial_sort(scored, scored + keep, scored + count,
                      [](const ScoredMove& a, const ScoredMove& b) {
                          return a.score > b.score;
                      });

    for (int i = 0; i < keep; ++i)
        out[i] = scored[i].pos;
    return keep;
}

int GameEngine::negamax(int depth, int alpha, int beta, Cell rootPlayer, SearchStats& stats)
{
    ++stats.nodes;
    if ((depth <= 2 || stats.nodes % 8 == 0) &&
        std::chrono::steady_clock::now() >= stats.deadline)
        return evaluate(rootPlayer);

    uint64_t hash = m_board.hash();

    int ttScore = 0;
    int ttMove  = -1;
    if (probeTT(hash, depth, alpha, beta, ttScore, ttMove))
        return ttScore;

    if (depth == 0 || m_board.isFull())
    {
        int s = evaluate(rootPlayer);
        storeTT(hash, depth, s, TTFlag::EXACT, -1);
        return s;
    }

    int moves[64];
    int moveCount = collectSearchMoves(m_currentPlayer, moves, false);
    if (moveCount == 0)
    {
        int s = evaluate(rootPlayer);
        storeTT(hash, depth, s, TTFlag::EXACT, -1);
        return s;
    }

    if (ttMove >= 0)
    {
        for (int i = 0; i < moveCount; ++i)
        {
            if (moves[i] == ttMove)
            {
                for (int j = i; j > 0; --j)
                    moves[j] = moves[j - 1];
                moves[0] = ttMove;
                break;
            }
        }
    }

    int  bestMove  = moves[0];
    int  best      = -INF_SCORE;
    int  origAlpha = alpha;
    Cell mover     = m_currentPlayer;

    for (int i = 0; i < moveCount; ++i)
    {
        int pos = moves[i];
        m_board.applyMove(pos, mover);

        WinReason wr = m_board.checkWinFast(mover, pos);
        int       val;
        if (wr != WinReason::NONE)
            val = terminalScore(mover, wr, depth, rootPlayer);
        else
        {
            m_currentPlayer = opponent(mover);
            val             = -negamax(depth - 1, -beta, -alpha, rootPlayer, stats);
            m_currentPlayer = mover;
        }

        m_board.undoMove();

        if (val > best)
        {
            best     = val;
            bestMove = pos;
        }
        alpha = std::max(alpha, best);
        if (alpha >= beta)
            break;
    }

    TTFlag flag = TTFlag::EXACT;
    if (best <= origAlpha)
        flag = TTFlag::UPPER;
    else if (best >= beta)
        flag = TTFlag::LOWER;
    storeTT(hash, depth, best, flag, bestMove);
    return best;
}

int GameEngine::chooseAiMove()
{
    Cell aiPlayer = m_currentPlayer;
    int  moves[64];
    int  moveCount = collectSearchMoves(aiPlayer, moves, true);
    if (moveCount == 0)
        return -1;

    SearchStats stats;
    stats.deadline = std::chrono::steady_clock::now() +
                     std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                         std::chrono::duration<double>(TIME_LIMIT_S));

    int bestMove       = moves[0];
    int bestScore      = -INF_SCORE;
    int completedDepth = 0;
    int startDepth     = 1;
    if (m_board.moveCount() < 2)
        startDepth = 10;
    else if (m_board.moveCount() < 8)
        startDepth = 6;

    for (int depth = startDepth; depth <= MAX_DEPTH; ++depth)
    {
        if (std::chrono::steady_clock::now() >= stats.deadline)
            break;

        int alpha      = -INF_SCORE;
        int beta       = INF_SCORE;
        int depthBest  = moves[0];
        int depthScore = -INF_SCORE;
        bool depthDone = true;

        for (int i = 0; i < moveCount; ++i)
        {
            if (std::chrono::steady_clock::now() >= stats.deadline)
            {
                depthDone = false;
                break;
            }

            int pos = moves[i];
            m_board.applyMove(pos, aiPlayer);

            WinReason wr = m_board.checkWinFast(aiPlayer, pos);
            int       val;
            if (wr != WinReason::NONE)
                val = terminalScore(aiPlayer, wr, depth, aiPlayer);
            else
            {
                m_currentPlayer = opponent(aiPlayer);
                val             = -negamax(depth - 1, -beta, -alpha, aiPlayer, stats);
                m_currentPlayer = aiPlayer;
            }

            m_board.undoMove();

            if (val > depthScore)
            {
                depthScore = val;
                depthBest  = pos;
            }
            alpha = std::max(alpha, depthScore);
        }

        if (depthDone && depthScore > -INF_SCORE / 2)
        {
            bestMove       = depthBest;
            bestScore      = depthScore;
            completedDepth = depth;
        }

        if (bestScore >= INF_SCORE - 1000)
            break;
    }

    m_lastSearchDepth = completedDepth;
    return bestMove;
}

void GameEngine::playAiMove()
{
    if (m_terminal || m_mode != GameMode::HUMAN_VS_AI)
        return;
    if (m_currentPlayer != Cell::WHITE)
        return;

    auto start = std::chrono::steady_clock::now();
    int  move  = chooseAiMove();
    auto end   = std::chrono::steady_clock::now();

    m_lastAiTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    if (move >= 0)
        playMove(move);
}

void GameEngine::updateSuggestion()
{
    if (m_terminal || m_mode != GameMode::HOTSEAT)
    {
        m_suggestedMove = -1;
        return;
    }

    auto start = std::chrono::steady_clock::now();
    m_suggestedMove = chooseAiMove();
    auto end = std::chrono::steady_clock::now();
    m_lastAiTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}
