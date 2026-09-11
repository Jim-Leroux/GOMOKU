#include "test.hpp"
#include "AI.hpp"
#include "GameEngine.hpp"
#include <chrono>

void test_ai_search()
{
    std::cout << "-- AI::findBestMove (alpha-beta search) --\n";

    // Depth 1: Black has an open four (four in a row, both ends free) —
    // there are two different winning completions, so just check that
    // whichever one comes back actually wins immediately.
    {
        GameEngine engine;
        engine.playMove(Board::index(9, 5));  engine.playMove(Board::index(0, 0));
        engine.playMove(Board::index(9, 6));  engine.playMove(Board::index(0, 1));
        engine.playMove(Board::index(9, 7));  engine.playMove(Board::index(0, 2));
        engine.playMove(Board::index(9, 8));  engine.playMove(Board::index(0, 3));

        AI ai(Cell::BLACK);
        int move = ai.findBestMove(engine, 1);
        CHECK(move == Board::index(9, 4) || move == Board::index(9, 9));

        engine.playMove(move);
        CHECK(engine.isTerminal());
        CHECK(engine.getWinner() == Cell::BLACK);
    }

    // Depth 2: White is one move away from an open four of their own
    // (which would be an unstoppable win the move after). Even though
    // evaluate() alone would rather extend Black's own shapes, looking
    // one more ply ahead must reveal that ignoring White's threat loses
    // next turn, so a 2-ply search has to block it now.
    {
        GameEngine engine;
        engine.playMove(Board::index(0, 0));    // Black: away from the threat
        engine.playMove(Board::index(9, 6));    // White
        engine.playMove(Board::index(0, 1));    // Black: away from the threat
        engine.playMove(Board::index(9, 7));    // White
        engine.playMove(Board::index(0, 2));    // Black: away from the threat
        engine.playMove(Board::index(9, 8));    // White: three in a row, both ends open

        AI ai(Cell::BLACK);
        int move = ai.findBestMove(engine, 2);
        CHECK(move == Board::index(9, 5) || move == Board::index(9, 9));
    }

    // Depth 4 on the reference position (three stones near center) must
    // complete inside the fast test suite — proof the search is fast enough
    // to iterate on without a separate throwaway benchmark.
    {
        GameEngine engine;
        engine.playMove(Board::index(9, 9));
        engine.playMove(Board::index(9, 10));
        engine.playMove(Board::index(8, 8));

        AI ai(Cell::WHITE);
        const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
        int move = ai.findBestMove(engine, 4);
        const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - t0).count();

        CHECK(move >= 0);
        CHECK(engine.isLegalMove(move));
        CHECK(ms < 2000);
    }
}
