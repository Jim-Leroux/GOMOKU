#include "test.hpp"
#include "GameEngine.hpp"

void test_capture()
{
    std::cout << "-- GameEngine::computeCaptures (task 2.2) --\n";

    // Flanking a pair removes exactly that pair, in all 8 directions.
    {
        constexpr int DIRS[8][2] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            { 0, -1},          { 0, 1},
            { 1, -1}, { 1, 0}, { 1, 1}
        };
        for (const int (&dir)[2] : DIRS)
        {
            // Each direction gets its own fresh engine, so reusing the same
            // center for all 8 is fine — nothing to keep apart.
            int row0 = 9, col0 = 9;
            int row1 = row0 + dir[0],     col1 = col0 + dir[1];
            int row2 = row0 + dir[0] * 2, col2 = col0 + dir[1] * 2;
            int row3 = row0 + dir[0] * 3, col3 = col0 + dir[1] * 3;

            GameEngine engine;
            engine.playMove(Board::index(row0, col0));  // Black: the flanking anchor
            engine.playMove(Board::index(row1, col1));  // White: first captured stone
            engine.playMove(Board::index(0, 0));        // Black: away from the pattern
            engine.playMove(Board::index(row2, col2));  // White: second captured stone
            CHECK(engine.getCaptureCount(Cell::BLACK) == 0); // not flanked yet

            engine.playMove(Board::index(row3, col3));  // Black closes the flank -> capture

            CHECK(engine.getBoard().isEmpty(Board::index(row1, col1)));
            CHECK(engine.getBoard().isEmpty(Board::index(row2, col2)));
            CHECK(engine.getCaptureCount(Cell::BLACK) == 2);
        }
    }

    // A run of exactly 3 opponent stones is never captured (must be exactly a pair).
    {
        GameEngine engine;
        engine.playMove(Board::index(0, 0));   // Black anchor
        engine.playMove(Board::index(0, 1));   // White
        engine.playMove(Board::index(18, 18)); // Black: away from the pattern
        engine.playMove(Board::index(0, 2));   // White
        engine.playMove(Board::index(18, 17)); // Black: away from the pattern
        engine.playMove(Board::index(0, 3));   // White
        engine.playMove(Board::index(0, 4));   // Black closes the flank on 3, not 2

        CHECK(engine.getBoard().get(0, 1) == Cell::WHITE);
        CHECK(engine.getBoard().get(0, 2) == Cell::WHITE);
        CHECK(engine.getBoard().get(0, 3) == Cell::WHITE);
        CHECK(engine.getCaptureCount(Cell::BLACK) == 0);
    }

    // "Playing into a capture" is always safe: if White completes the O-W-W-O
    // shape by filling the *middle* gap, nothing is captured — only the
    // flanking player's own move can trigger a capture, never the trapped
    // player's move that merely completes the sandwich.
    {
        GameEngine engine;
        int p0 = Board::index(2, 2), p1 = Board::index(2, 3);
        int p2 = Board::index(2, 4), p3 = Board::index(2, 5);

        engine.playMove(p0);     // Black
        engine.playMove(p1);     // White
        engine.playMove(p3);     // Black (note: skips p2 on purpose)
        engine.playMove(p2);     // White completes B W W B by playing the middle

        CHECK(engine.getBoard().get(p1) == Cell::WHITE); // still there
        CHECK(engine.getBoard().get(p2) == Cell::WHITE); // still there
        CHECK(engine.getCaptureCount(Cell::BLACK) == 0);
        CHECK(engine.getCaptureCount(Cell::WHITE) == 0);
        CHECK(!engine.isTerminal());
    }

    // Win by 10 captured stones (5 pairs), and not a move sooner.
    // Each pair lives on its own row, spaced 4 apart so the 5 anchor stones
    // in column 0 (and the 5 flanking stones in column 3) are never
    // contiguous — otherwise this would accidentally also be a vertical
    // 5-in-a-row and we'd be testing the wrong rule.
    {
        GameEngine engine;
        for (int pair = 0; pair < 5; ++pair)
        {
            int row = pair * 4;
            int row0 = row, col0 = 0;
            int row1 = row, col1 = 1;
            int row2 = row, col2 = 2;
            int row3 = row, col3 = 3;

            engine.playMove(Board::index(row0, col0));   // Black anchor
            engine.playMove(Board::index(row1, col1));   // White
            // Col 18, spaced rows — must not form a 5-in-a-row of distractors.
            engine.playMove(Board::index(pair * 4 + 1, 18));
            engine.playMove(Board::index(row2, col2));   // White
            bool lastPair = (pair == 4);
            engine.playMove(Board::index(row3, col3));   // Black closes the flank

            if (!lastPair)
            {
                CHECK(!engine.isTerminal());
                engine.playMove(Board::index(pair * 4 + 2, 18)); // White: away from the pattern
            }
        }

        CHECK(engine.getCaptureCount(Cell::BLACK) == 10);
        CHECK(engine.isTerminal());
        CHECK(engine.getWinner() == Cell::BLACK);
    }
}
