#include "test.hpp"
#include "GameEngine.hpp"
#include <stdexcept>

namespace
{
    bool throwsOnPlay(GameEngine& engine, int pos)
    {
        try { engine.playMove(pos); }
        catch (const std::invalid_argument&) { return true; }
        return false;
    }
}

void test_double_three()
{
    std::cout << "-- GameEngine::checkDoubleThree (task 2.3) --\n";

    // A cross of two open pairs: Black already has (9,7)-(9,8) horizontally
    // and (7,9)-(8,9) vertically. Playing (9,9) would complete an open three
    // on BOTH axes at once — the classic double-three — and must be rejected.
    {
        GameEngine engine;
        engine.playMove(Board::index(9, 7));  engine.playMove(Board::index(18, 0));
        engine.playMove(Board::index(9, 8));  engine.playMove(Board::index(18, 1));
        engine.playMove(Board::index(7, 9));  engine.playMove(Board::index(18, 2));
        engine.playMove(Board::index(8, 9));  engine.playMove(Board::index(18, 3));

        int critical = Board::index(9, 9);
        CHECK(!engine.isLegalMove(critical));
        CHECK(throwsOnPlay(engine, critical));
    }

    // Sanity check: a *single* open three is completely legal — only the
    // double is banned.
    {
        GameEngine engine;
        engine.playMove(Board::index(0, 7));  engine.playMove(Board::index(18, 0));
        engine.playMove(Board::index(0, 8));  engine.playMove(Board::index(18, 1));

        CHECK(engine.isLegalMove(Board::index(0, 9)));
        engine.playMove(Board::index(0, 9)); // must not throw
        CHECK(!engine.isTerminal());
    }

    // Same idea, but one axis is a *broken* three (the gap sits in the
    // middle of the window, not at an edge): Black has (5,6) and (5,8) with
    // a gap at (5,7). Playing (5,5) makes the horizontal window
    // [(5,5)(5,6)(5,7)(5,8)] read B,B,_,B — a broken three — while also
    // completing a plain vertical three via (3,5)-(4,5).
    {
        GameEngine engine;
        engine.playMove(Board::index(5, 6));  engine.playMove(Board::index(18, 0));
        engine.playMove(Board::index(5, 8));  engine.playMove(Board::index(18, 1));
        engine.playMove(Board::index(3, 5));  engine.playMove(Board::index(18, 2));
        engine.playMove(Board::index(4, 5));  engine.playMove(Board::index(18, 3));

        int critical = Board::index(5, 5);
        CHECK(!engine.isLegalMove(critical));
    }

    // Capture exception: Black sets up the same kind of cross (open three on
    // row 10 and column 10, crossing at (10,10)), but this time (10,10) also
    // flanks a White pair at (9,9)-(8,8) against a Black anchor at (7,7).
    // The move both captures AND would-be a double-three — it must stay legal.
    {
        GameEngine engine;

        engine.playMove(Board::index(7, 7));    // Black: capture anchor
        engine.playMove(Board::index(8, 8));    // White: capturable stone #1
        engine.playMove(Board::index(10, 8));   // Black: horizontal leg
        engine.playMove(Board::index(9, 9));    // White: capturable stone #2
        engine.playMove(Board::index(10, 9));   // Black: horizontal leg
        engine.playMove(Board::index(0, 0));    // White: away from the pattern
        engine.playMove(Board::index(8, 10));   // Black: vertical leg
        engine.playMove(Board::index(0, 1));    // White: away from the pattern
        engine.playMove(Board::index(9, 10));   // Black: vertical leg
        engine.playMove(Board::index(0, 2));    // White: away from the pattern

        int critical = Board::index(10, 10);
        CHECK(engine.isLegalMove(critical)); // legal: it captures

        engine.playMove(critical); // must not throw
        CHECK(engine.getBoard().isEmpty(Board::index(8, 8)));
        CHECK(engine.getBoard().isEmpty(Board::index(9, 9)));
        CHECK(engine.getCaptureCount(Cell::BLACK) == 2);
    }
}
