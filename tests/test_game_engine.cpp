#include "test.hpp"
#include "GameEngine.hpp"

void test_game_engine_basics()
{
    std::cout << "-- GameEngine (basics) --\n";

    GameEngine engine;

    // Fresh engine: Black to play, nothing on the board yet
    CHECK(engine.getCurrentPlayer() == Cell::BLACK);
    CHECK(!engine.isTerminal());

    // isLegalMove currently only checks "empty cell" + "game not over" —
    // the real rules land in the next four chapters.
    int center = Board::index(9, 9);
    CHECK(engine.isLegalMove(center));

    engine.playMove(center);
    CHECK(engine.getBoard().get(center) == Cell::BLACK);
    CHECK(engine.getCurrentPlayer() == Cell::WHITE); // turn alternates
    CHECK(!engine.isLegalMove(center));               // occupied now

    int other = Board::index(9, 10);
    engine.playMove(other);

    // undoMove rewinds both the board and whose turn it is
    engine.undoMove();
    CHECK(engine.getCurrentPlayer() == Cell::WHITE);
    engine.undoMove();
    CHECK(engine.getCurrentPlayer() == Cell::BLACK);
    CHECK(engine.getMoveCount() == 0);

    // reset() clears everything
    engine.playMove(center);
    engine.reset();
    CHECK(engine.getMoveCount() == 0);
    CHECK(engine.getCurrentPlayer() == Cell::BLACK);
}
