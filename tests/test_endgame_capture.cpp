#include "test.hpp"
#include "GameEngine.hpp"

namespace
{
    // Horizontal Black 5-in-a-row on `lineRow`, cols `startCol`..`startCol+4`.
    // White can capture the stone at `threatenedIndex` (0..4) on the next move.
    //
    // pairRole 0 → stoneIndex 0: EMPTY, stone, partner, opp
    // pairRole 1 → stoneIndex 1: EMPTY, partner, stone, opp
    // stoneIndex 2..4 cannot put this player stone in a capturable pair
    // with the row1–row4 geometry, so each of the five alignment stones is
    // threatened instead (each is found at slot 0 or 1 for THAT stone).
    void playBreakableFive(GameEngine& engine, int threatenedIndex, int pairRole)
    {
        const int lineRow = 10;
        const int startCol = 7;
        const int threatenedCol = startCol + threatenedIndex;
        const int partnerRow = (pairRole == 0) ? lineRow - 1 : lineRow + 1;
        const int oppRow     = (pairRole == 0) ? lineRow - 2 : lineRow - 1;
        int distractorCol = 0;

        engine.playMove(Board::index(partnerRow, threatenedCol)); // Black: extra pair stone
        engine.playMove(Board::index(oppRow, threatenedCol));     // White: capture anchor

        for (int i = 0; i < 5; ++i)
        {
            engine.playMove(Board::index(lineRow, startCol + i));
            if (i < 4)
                engine.playMove(Board::index(0, distractorCol++));
        }
    }
}

void test_endgame_capture()
{
    std::cout << "-- GameEngine::checkEndgameCapture (task 2.4) --\n";

    // An unbreakable 5-in-a-row still wins immediately (no capture nearby).
    {
        GameEngine engine;
        engine.playMove(Board::index(10, 0)); engine.playMove(Board::index(0, 0));
        engine.playMove(Board::index(10, 1)); engine.playMove(Board::index(0, 1));
        engine.playMove(Board::index(10, 2)); engine.playMove(Board::index(0, 2));
        engine.playMove(Board::index(10, 3)); engine.playMove(Board::index(0, 3));
        engine.playMove(Board::index(10, 4)); // completes the line

        CHECK(engine.isTerminal());
        CHECK(engine.getWinner() == Cell::BLACK);
    }

    // A 5-in-a-row that CAN be broken by an immediate capture does not end
    // the game. Setup: Black plays row 5, cols 0-4. Black also has an extra
    // stone at (4,4), and White already anchors (3,4) — so the moment Black
    // completes the line at (5,4), White could reply at (6,4) to capture
    // (5,4) and (4,4), breaking the line. The game must stay open.
    {
        GameEngine engine;
        engine.playMove(Board::index(5, 0));
        engine.playMove(Board::index(3, 4));   // White's future capture anchor
        engine.playMove(Board::index(5, 1));
        engine.playMove(Board::index(18, 0));  // White: away from the pattern
        engine.playMove(Board::index(5, 2));
        engine.playMove(Board::index(18, 1));  // White: away from the pattern
        engine.playMove(Board::index(5, 3));
        engine.playMove(Board::index(18, 2));  // White: away from the pattern
        engine.playMove(Board::index(4, 4));   // extra Black stone, sets up the trap
        engine.playMove(Board::index(18, 3));  // White: away from the pattern
        engine.playMove(Board::index(5, 4));   // completes the 5-in-a-row

        CHECK(!engine.isTerminal()); // breakable -> doesn't end the game yet
        CHECK(engine.getWinner() == Cell::EMPTY);

        // Now White actually plays the capture, breaking the line for real.
        int capturePos = Board::index(6, 4);
        CHECK(engine.isLegalMove(capturePos));
        engine.playMove(capturePos);

        CHECK(engine.getBoard().isEmpty(Board::index(5, 4)));
        CHECK(engine.getBoard().isEmpty(Board::index(4, 4)));
        CHECK(engine.getCaptureCount(Cell::WHITE) == 2);
        CHECK(!engine.isTerminal()); // still going: the line is gone
    }

    // Each of the five alignment stones, in both pair orientations that
    // row1–row4 can actually match (stoneIndex 0 and 1). Completing move is
    // always the last cell of the line, so indices 0–3 also prove we do not
    // only inspect the stone that was just placed.
    for (int threatenedIndex = 0; threatenedIndex < 5; ++threatenedIndex)
    {
        for (int pairRole = 0; pairRole < 2; ++pairRole)
        {
            GameEngine engine;
            playBreakableFive(engine, threatenedIndex, pairRole);
            CHECK(!engine.isTerminal());
            CHECK(engine.getWinner() == Cell::EMPTY);
        }
    }

    // An extra aligner stone beside the line is not enough: without an
    // opponent anchor at row4, White cannot capture, so Black wins.
    {
        GameEngine engine;
        engine.playMove(Board::index(9, 11));  engine.playMove(Board::index(0, 0));
        engine.playMove(Board::index(10, 7));  engine.playMove(Board::index(0, 2));
        engine.playMove(Board::index(10, 8));  engine.playMove(Board::index(0, 4));
        engine.playMove(Board::index(10, 9));  engine.playMove(Board::index(0, 6));
        engine.playMove(Board::index(10, 10)); engine.playMove(Board::index(0, 8));
        engine.playMove(Board::index(10, 11)); // completes (10,7–11), partner at (9,11), (8,11) empty

        CHECK(engine.isTerminal());
        CHECK(engine.getWinner() == Cell::BLACK);
    }

    // White sitting on one side of a single line stone is not a pair either
    // (row2 is empty). Pattern _ _ P O must not be treated as a capture.
    {
        GameEngine engine;
        engine.playMove(Board::index(10, 7));  engine.playMove(Board::index(9, 11));
        engine.playMove(Board::index(10, 8));  engine.playMove(Board::index(0, 0));
        engine.playMove(Board::index(10, 9));  engine.playMove(Board::index(0, 1));
        engine.playMove(Board::index(10, 10)); engine.playMove(Board::index(0, 2));
        engine.playMove(Board::index(10, 11));

        CHECK(engine.isTerminal());
        CHECK(engine.getWinner() == Cell::BLACK);
    }
}
