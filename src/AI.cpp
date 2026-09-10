#include "AI.hpp"

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
};

int AI::evaluate(const Board& board) const
{
    Cell ai = m_aiPlayer;
    Cell human = opponent(ai);
    int score = captureScore(board.getCaptureCount(ai))
              - captureScore(board.getCaptureCount(human));
    for (Cell side : { ai, human })
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
