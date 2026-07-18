#include "test.hpp"
#include "GameEngine.hpp"

namespace
{
    // Plays `player` at every position in `cells`, interleaving a harmless
    // White move so turn order never gets in the way of the scenario we're
    // actually testing.
    void playRun(GameEngine& engine, const std::vector<int>& cells, int distractorRow)
    {
        int distractorCol = 0;
        for (int pos : cells)
        {
            engine.playMove(pos); // BLACK
            if (!engine.isTerminal())
                engine.playMove(Board::index(distractorRow, distractorCol++)); // WHITE, far away
        }
    }
}

void test_alignment()
{
    std::cout << "-- GameEngine::checkAlignment (task 2.1) --\n";

    // Horizontal 5-in-a-row wins
    {
        GameEngine engine;
        std::vector<int> row;
        for (int col = 0; col < 5; ++col) row.push_back(Board::index(0, col));
        playRun(engine, row, 17);

        CHECK(engine.isTerminal());
        CHECK(engine.getWinner() == Cell::BLACK);
    }

    // Vertical 5-in-a-row wins
    {
        GameEngine engine;
        std::vector<int> col;
        for (int row = 0; row < 5; ++row) col.push_back(Board::index(row, 0));
        playRun(engine, col, 17);

        CHECK(engine.isTerminal());
        CHECK(engine.getWinner() == Cell::BLACK);
    }

    // Diagonal ↘ wins
    {
        GameEngine engine;
        std::vector<int> diag;
        for (int i = 0; i < 5; ++i) diag.push_back(Board::index(i, i));
        playRun(engine, diag, 17);

        CHECK(engine.isTerminal());
        CHECK(engine.getWinner() == Cell::BLACK);
    }

    // Diagonal ↙ (anti-diagonal) wins
    {
        GameEngine engine;
        std::vector<int> diag;
        for (int i = 0; i < 5; ++i) diag.push_back(Board::index(i, 10 - i));
        playRun(engine, diag, 17);

        CHECK(engine.isTerminal());
        CHECK(engine.getWinner() == Cell::BLACK);
    }

    // 4-in-a-row must NOT win
    {
        GameEngine engine;
        std::vector<int> row;
        for (int col = 0; col < 4; ++col) row.push_back(Board::index(0, col));
        playRun(engine, row, 17);

        CHECK(!engine.isTerminal());
    }

    // The winning stone doesn't have to be at the end of the line — this
    // exercises the "look both ways from pos" part of checkAlignment.
    {
        GameEngine engine;
        // Black plays columns 1,2,3,4 first (no win), then fills column 0 last.
        std::vector<int> order = { Board::index(0, 1), Board::index(0, 2),
                                    Board::index(0, 3), Board::index(0, 4),
                                    Board::index(0, 0) };
        playRun(engine, order, 17);

        CHECK(engine.isTerminal());
        CHECK(engine.getWinner() == Cell::BLACK);
    }
}
