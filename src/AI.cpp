#include "AI.hpp"
#include <algorithm>
#include <limits>

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

std::vector<int> AI::getBestMoves(GameEngine& engine)
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
    for (const std::pair<int, int>& rankedMove : rankedMoves)
    {
        if (!engine.isLegalMove(rankedMove.second))
            continue;
        bestMoves.push_back(rankedMove.second);
        if (static_cast<int>(bestMoves.size()) >= MAX_CANDIDATES)
            break;
    }
    return bestMoves;
}

int AI::minMax(GameEngine& engine, int depth, int alpha, int beta, bool maximizing)
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
    std::vector<int> moves = getBestMoves(engine);
    if (moves.empty())
        return evaluate(engine.getBoard());
    if (maximizing)
    {
        int bestScore = std::numeric_limits<int>::min();
        for (int move : moves)
        {
            engine.playMove(move);
            bestScore = std::max(bestScore, minMax(engine, depth - 1, alpha, beta, false));
            engine.undoMove();
            alpha = std::max(alpha, bestScore);
            if (alpha >= beta)
                break;
        }
        return bestScore;
    }
    else
    {
        int bestScore = std::numeric_limits<int>::max();
        for (int move : moves)
        {
            engine.playMove(move);
            bestScore = std::min(bestScore, minMax(engine, depth - 1, alpha, beta, true));
            engine.undoMove();
            beta = std::min(beta, bestScore);
            if (alpha >= beta)
                break;
        }
        return bestScore;
    }
}

int AI::findBestMove(GameEngine& engine, int depth)
{
    std::vector<int> moves = getBestMoves(engine);
    if (moves.empty())
        return -1;
    int bestMove = moves.front();
    int bestScore = std::numeric_limits<int>::min();
    int alpha = std::numeric_limits<int>::min();
    const int beta = std::numeric_limits<int>::max();
    for (int move : moves)
    {
        engine.playMove(move);
        int score = minMax(engine, depth - 1, alpha, beta, false);
        engine.undoMove();
        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }
        alpha = std::max(alpha, bestScore);
    }
    return bestMove;
}
