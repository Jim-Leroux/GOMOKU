#include "AI.hpp"

#include <algorithm>
#include <limits>
#include <random>
#include <utility>

namespace
{
    constexpr int AXES[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };

    int shapeScore(int count, int openEnds)
    {
        if (count >= 5)
            return 100000;
        if (count == 4)
            return openEnds == 2 ? 15000 : openEnds == 1 ? 2000 : 0;
        if (count == 3)
            return openEnds == 2 ? 1000 : openEnds == 1 ? 150 : 0;
        if (count == 2)
            return openEnds == 2 ? 50 : openEnds == 1 ? 10 : 0;
        return 0;
    }
    
    int captureScore(int stones)
    {
        int pairs = stones / 2;
        return pairs * pairs * 200;
    }

    constexpr int CAP_CLAMP = 10;
    uint64_t Z_STONE[BOARD_CELLS][2];
    uint64_t Z_SIDE;
    uint64_t Z_CAP[2][CAP_CLAMP + 1];
    bool Z_READY = false;

    void initZobrist()
    {
        if (Z_READY)
            return;
        std::mt19937_64 rng;
        for (int cellIndex = 0; cellIndex < BOARD_CELLS; ++cellIndex)
            for (int playerIndex = 0; playerIndex < 2; ++playerIndex)
                Z_STONE[cellIndex][playerIndex] = rng();
        Z_SIDE = rng();
        for (int playerIndex = 0; playerIndex < 2; ++playerIndex)
            for (int captureCount = 0; captureCount <= CAP_CLAMP; ++captureCount)
                Z_CAP[playerIndex][captureCount] = rng();
        Z_READY = true;
    }

    inline int colorIndex(Cell player)
    {
        if (player == Cell::BLACK)
            return 0;
        else
            return 1;
    }

    int countStonesInDirection(const Board& board, int row, int col, int deltaRow, int deltaCol, Cell player)
    {
        int n = 0;
        int nextRow = row + deltaRow, nextCol = col + deltaCol;
        while (Board::isValid(nextRow, nextCol) && board.get(nextRow, nextCol) == player)
        {
            ++n;
            nextRow += deltaRow, nextCol += deltaCol;
        }
        return n;
    }

    bool isOpenEndAfterStones(const Board& board, int row, int col, int deltaRow, int deltaCol, int stoneCount)
    {
        int nextRow = row + deltaRow * (stoneCount + 1), nextCol = col + deltaCol * (stoneCount + 1);
        return Board::isValid(nextRow, nextCol) && board.get(nextRow, nextCol) == Cell::EMPTY;
    }
};

int AI::evaluate(const Board& board) const
{
    Cell ai = m_aiPlayer;
    Cell opp = opponent(ai);
    int score = captureScore(board.getCaptureCount(ai))
              - captureScore(board.getCaptureCount(opp));
    for (Cell side : { ai, opp })
    {
        const int polarity = side == ai ? 1 : -1;
        for (int i = 0; i < BOARD_CELLS; ++i)
        {
            if (board.get(i) != side)
                continue;
            int row = Board::row(i), col = Board::col(i);
            for (const int (&axis)[2] : AXES)
            {
                int deltaRow = axis[0], deltaCol = axis[1];
                int prevRow = row - deltaRow, prevCol = col - deltaCol;
                if (Board::isValid(prevRow, prevCol) && board.get(prevRow, prevCol) == side)
                    continue;
                int count = 0;
                int nextRow = row, nextCol = col;
                while (Board::isValid(nextRow, nextCol) && board.get(nextRow, nextCol) == side)
                {
                    ++count;
                    nextRow += deltaRow, nextCol += deltaCol;
                }
                int openEnds = 0;
                if (Board::isValid(prevRow, prevCol) && board.get(prevRow, prevCol) == Cell::EMPTY)
                    ++openEnds;
                if (Board::isValid(nextRow, nextCol) && board.get(nextRow, nextCol) == Cell::EMPTY)
                    ++openEnds;
                score += polarity * shapeScore(count, openEnds);
            }
        }
    }
    return score;
}

std::vector<int> AI::nearbyEmptyCells(GameEngine& engine) const
{
    const Board& board = engine.getBoard();
    if (board.moveCount() == 0)
        return { Board::index(BOARD_SIZE / 2, BOARD_SIZE / 2) };
    constexpr int RADIUS = 2;
    std::vector<bool> seen(BOARD_CELLS, false);
    std::vector<int> moves;
    for (int i = 0; i < BOARD_CELLS; ++i)
    {
        if (board.get(i) == Cell::EMPTY)
            continue;
        int row = Board::row(i), col = Board::col(i);
        for (int deltaRow = -RADIUS; deltaRow <= RADIUS; ++deltaRow)
        {
            for (int deltaCol = -RADIUS; deltaCol <= RADIUS; ++deltaCol)
            {
                int nextRow = row + deltaRow, nextCol = col + deltaCol;
                if (!Board::isValid(nextRow, nextCol))
                    continue;
                int nextIndex = Board::index(nextRow, nextCol);
                if (seen[nextIndex] || board.get(nextIndex) != Cell::EMPTY)
                    continue;
                seen[nextIndex] = true;
                moves.push_back(nextIndex);
            }
        }
    }
    return moves;
}

int AI::quickMoveScore(const Board& board, int move, Cell player) const
{
    Cell opp = opponent(player);
    int row = Board::row(move), col = Board::col(move);
    int score = 0;
    for (const int (&axis)[2] : AXES)
    {
        int deltaRow = axis[0], deltaCol = axis[1];
        int stonesFwd = countStonesInDirection(board, row, col, deltaRow, deltaCol, player);
        int stonesBack = countStonesInDirection(board, row, col, -deltaRow, -deltaCol, player);
        int openEnds = (isOpenEndAfterStones(board, row, col, deltaRow, deltaCol, stonesFwd) ? 1 : 0)
                     + (isOpenEndAfterStones(board, row, col, -deltaRow, -deltaCol, stonesBack) ? 1 : 0);
        score += shapeScore(stonesFwd + stonesBack + 1, openEnds);
        int oppStones = std::max(countStonesInDirection(board, row, col, deltaRow, deltaCol, opp),
                                 countStonesInDirection(board, row, col, -deltaRow, -deltaCol, opp));
        if (oppStones >= 2)
            score += shapeScore(oppStones + 1, 1);
    }
    return score;
}

std::vector<int> AI::getBestMoves(GameEngine& engine, int priorityMove)
{
    const Board& board = engine.getBoard();
    Cell player = engine.getCurrentPlayer();
    std::vector<int> cells = nearbyEmptyCells(engine);
    std::vector<std::pair<int, int>> rankedMoves; // <score, move>
    rankedMoves.reserve(cells.size());
    for (int move : cells)
        rankedMoves.push_back({ quickMoveScore(board, move, player), move });
    std::sort(rankedMoves.begin(), rankedMoves.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b)
    {
        return a.first > b.first;
    });
    std::vector<int> bestMoves;
    bestMoves.reserve(MAX_CANDIDATES);
    if (priorityMove >= 0 && engine.isLegalMove(priorityMove))
        bestMoves.push_back(priorityMove);
    for (const std::pair<int, int>& rankedMove : rankedMoves)
    {
        if (rankedMove.second == priorityMove)
            continue;
        if (!engine.isLegalMove(rankedMove.second))
            continue;
        bestMoves.push_back(rankedMove.second);
        if (static_cast<int>(bestMoves.size()) >= MAX_CANDIDATES)
            break;
    }
    return bestMoves;
}

int AI::minMax(GameEngine& engine, int depth,
    int alpha, int beta, bool maximizing,
    Clock::time_point deadline)
{
    if (engine.isTerminal())
    {
        Cell winner = engine.getWinner();
        if (winner == m_aiPlayer)
            return WIN_SCORE;
        if (winner == opponent(m_aiPlayer))
            return -WIN_SCORE;
        return 0;
    }
    if (depth == 0)
        return evaluate(engine.getBoard());

    const int alphaOrig = alpha;
    const int betaOrig = beta;

    uint64_t key = positionKey(engine);
    TTEntry& entry = m_tt[key & (TT_SIZE - 1)];
    int priorityMove = -1;
    if (entry.key == key && entry.depth != -1)
    {
        priorityMove = entry.bestMove;
        if (entry.depth >= depth)
        {
            if (entry.flag == TT_EXACT)
                return entry.score;
            if (entry.flag == TT_LOWER)
                alpha = std::max(alpha, entry.score);
            else
                beta = std::min(beta, entry.score);
            if (alpha >= beta)
                return entry.score;
        }
    }

    std::vector<int> moves = getBestMoves(engine, priorityMove);
    if (moves.empty())
        return evaluate(engine.getBoard());
    int bestScore;
    int bestMove = moves.front();
    if (maximizing)
    {
        bestScore = std::numeric_limits<int>::min();
        for (int move : moves)
        {
            if (Clock::now() >= deadline)
                break;
            playHashed(engine, move);
            int score = minMax(engine, depth - 1, alpha, beta, false, deadline);
            undoHashed(engine);
            if (score > bestScore)
            {
                bestScore = score;
                bestMove = move;
            }
            alpha = std::max(alpha, bestScore);
            if (alpha >= beta)
                break;
        }
    }
    else
    {
        bestScore = std::numeric_limits<int>::max();
        for (int move : moves)
        {
            if (Clock::now() >= deadline)
                break;
            playHashed(engine, move);
            int score = minMax(engine, depth - 1, alpha, beta, true, deadline);
            undoHashed(engine);
            if (score < bestScore)
            {
                bestScore = score;
                bestMove = move;
            }
            beta = std::min(beta, bestScore);
            if (alpha >= beta)
                break;
        }
    }
    if (Clock::now() < deadline)
    {
        uint8_t flag = bestScore <= alphaOrig ? TT_UPPER
                     : bestScore >= betaOrig ? TT_LOWER
                     : TT_EXACT;
        entry.key = key;
        entry.depth = depth;
        entry.score = bestScore;
        entry.bestMove = bestMove;
        entry.flag = flag;
    }
    return bestScore;
}

int AI::findBestMove(GameEngine& engine, int depth)
{
    initSearch(engine);
    std::vector<int> moves = getBestMoves(engine, -1);
    if (moves.empty())
        return -1;
    int bestMove = moves.front();
    int bestScore = std::numeric_limits<int>::min();
    int alpha = std::numeric_limits<int>::min();
    const int beta = std::numeric_limits<int>::max();
    for (int move : moves)
    {
        playHashed(engine, move);
        int score = minMax(engine, depth - 1, alpha, beta, false);
        undoHashed(engine);
        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }
        alpha = std::max(alpha, bestScore);
    }
    return bestMove;
}

int AI::findBestMoveTimed(GameEngine& engine, std::chrono::milliseconds budget)
{
    Clock::time_point start = Clock::now();
    Clock::time_point deadline = start + budget;
    initSearch(engine);
    std::vector<int> firstMoves = getBestMoves(engine, -1);
    if (firstMoves.empty())
    {
        m_lastStats = { 0, 0 };
        m_lastSearchDepth = 0;
        m_lastThinkingTimeMs = 0;
        return -1;
    }
    int bestMove = firstMoves.front();
    int reachedDepth = 0;
    for (int depth = 1; depth <= MAX_DEPTH; ++depth)
    {
        int alpha = std::numeric_limits<int>::min();
        const int beta = std::numeric_limits<int>::max();
        int depthBestMove = -1;
        int depthBestValue = std::numeric_limits<int>::min();
        bool depthCompleted = true;
        std::vector<int> moves = getBestMoves(engine, bestMove);
        depthBestMove = moves.front();
        for (int move : moves)
        {
            if (Clock::now() >= deadline)
            {
                depthCompleted = false;
                break;
            }
            playHashed(engine, move);
            int value = minMax(engine, depth - 1, alpha, beta, false, deadline);
            undoHashed(engine);
            if (Clock::now() >= deadline)
            {
                depthCompleted = false;
                break;
            }
            if (value > depthBestValue)
            {
                depthBestValue = value;
                depthBestMove = move;
            }
            alpha = std::max(alpha, depthBestValue);
        }
        if (!depthCompleted)
            break;
        bestMove = depthBestMove;
        reachedDepth = depth;
        if (depthBestValue >= WIN_SCORE)
            break;
    }
    m_lastStats.depthReached = reachedDepth;
    m_lastStats.elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count();
    m_lastSearchDepth = reachedDepth;
    m_lastThinkingTimeMs = m_lastStats.elapsedMs;
    return bestMove;
}

void AI::initSearch(GameEngine& engine)
{
    initZobrist();
    for (TTEntry& e : m_tt)
        e = TTEntry{};
    const Board& board = engine.getBoard();
    m_hash = 0;
    for (int cellIndex = 0; cellIndex < BOARD_CELLS; ++cellIndex)
    {
        Cell occupant = board.get(cellIndex);
        if (occupant != Cell::EMPTY)
            m_hash ^= Z_STONE[cellIndex][colorIndex(occupant)];
    }
    if (engine.getCurrentPlayer() == Cell::WHITE)
        m_hash ^= Z_SIDE;
}

void AI::playHashed(GameEngine& engine, int move)
{
    Cell mover = engine.getCurrentPlayer();
    engine.playMove(move);
    const Move& mv = engine.getBoard().history().back();
    m_hash ^= Z_STONE[mv.pos][colorIndex(mover)];
    for (int cap : mv.captured)
        m_hash ^= Z_STONE[cap][colorIndex(opponent(mover))];
    m_hash ^= Z_SIDE;
}

void AI::undoHashed(GameEngine& engine)
{
    const Move& mv = engine.getBoard().history().back();
    Cell mover = mv.player;
    m_hash ^= Z_STONE[mv.pos][colorIndex(mover)];
    for (int cap : mv.captured)
        m_hash ^= Z_STONE[cap][colorIndex(opponent(mover))];
    m_hash ^= Z_SIDE;
    engine.undoMove();
}

uint64_t AI::positionKey(GameEngine& engine) const
{
    int bCap = std::min(engine.getCaptureCount(Cell::BLACK), CAP_CLAMP);
    int wCap = std::min(engine.getCaptureCount(Cell::WHITE), CAP_CLAMP);
    return m_hash ^ Z_CAP[0][bCap] ^ Z_CAP[1][wCap];
}
