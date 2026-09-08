#include "test.hpp"
#include "GameEngine.hpp"

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
}
