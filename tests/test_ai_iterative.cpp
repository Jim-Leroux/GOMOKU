#include "test.hpp"
#include "AI.hpp"
#include "GameEngine.hpp"
#include <chrono>

void test_ai_iterative_deepening()
{
    std::cout << "-- AI::findBestMoveTimed (iterative deepening) --\n";

    {
        GameEngine engine;
        engine.playMove(Board::index(9, 5));  engine.playMove(Board::index(0, 0));
        engine.playMove(Board::index(9, 6));  engine.playMove(Board::index(0, 1));
        engine.playMove(Board::index(9, 7));  engine.playMove(Board::index(0, 2));
        engine.playMove(Board::index(9, 8));  engine.playMove(Board::index(0, 3));

        AI ai(Cell::BLACK);
        int move = ai.findBestMoveTimed(engine, std::chrono::milliseconds(50));
        CHECK(move == Board::index(9, 4) || move == Board::index(9, 9));
        CHECK(ai.lastStats().depthReached >= 1);
    }

    {
        GameEngine engine;
        engine.playMove(Board::index(9, 9));
        engine.playMove(Board::index(9, 10));
        engine.playMove(Board::index(8, 8));

        AI ai(Cell::WHITE);
        const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
        int move = ai.findBestMoveTimed(engine, std::chrono::milliseconds(50));
        const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - t0).count();

        CHECK(move >= 0);
        CHECK(engine.isLegalMove(move));
        CHECK(ms < 2000);
        CHECK(ai.lastStats().depthReached >= 1);
    }

    {
        GameEngine engine;
        engine.playMove(Board::index(9, 9));
        engine.playMove(Board::index(9, 10));
        engine.playMove(Board::index(8, 8));

        AI aiShort(Cell::WHITE);
        aiShort.findBestMoveTimed(engine, std::chrono::milliseconds(15));
        int shortDepth = aiShort.lastStats().depthReached;

        AI aiLong(Cell::WHITE);
        aiLong.findBestMoveTimed(engine, std::chrono::milliseconds(80));
        int longDepth = aiLong.lastStats().depthReached;

        CHECK(longDepth >= shortDepth);
    }
}
