#pragma once

#include "GameEngine.hpp"
#include <iostream>
#include <string>

namespace Test
{

    inline int g_passed = 0;
    inline int g_failed = 0;

    inline void check(bool cond, const char* expr, const char* file, int line)
    {
        if (cond)
        {
            ++g_passed;
            return;
        }
        ++g_failed;
        std::cerr << "Test failed: " << expr << " in " << file << ":" << line << std::endl;
    }

    inline int summary()
    {
        std::cout << std::endl << "Tests summary: " << g_passed << " passed, " << g_failed << " failed" << std::endl;
        return g_failed > 0 ? 1 : 0;
    }

    inline bool touchesOccupied(const Board& board, int idx)
    {
        const int r = Board::row(idx);
        const int c = Board::col(idx);
        for (int dr = -1; dr <= 1; ++dr)
        {
            for (int dc = -1; dc <= 1; ++dc)
            {
                if (dr == 0 && dc == 0)
                    continue;
                const int nr = r + dr;
                const int nc = c + dc;
                if (Board::isValid(nr, nc)
                    && !board.isEmpty(Board::index(nr, nc)))
                    return true;
            }
        }
        return false;
    }

    inline void filler(GameEngine& engine)
    {
        const Board& board = engine.getBoard();
        int fallback = -1;
        for (int i = 0; i < BOARD_CELLS; ++i)
        {
            if (!engine.isLegalMove(i))
                continue;
            if (!touchesOccupied(board, i))
            {
                engine.playMove(i);
                return;
            }
            if (fallback < 0)
                fallback = i;
        }
        if (fallback >= 0)
            engine.playMove(fallback);
    }
}

#define CHECK(cond) Test::check(cond, #cond, __FILE__, __LINE__)
