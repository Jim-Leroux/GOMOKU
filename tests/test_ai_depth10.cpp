#include "test.hpp"
#include "AI.hpp"
#include "GameEngine.hpp"
#include <chrono>

void test_ai_depth10()
{
    std::cout << "-- AI depth-10 performance (<=500ms) --\n";

    GameEngine engine;
    engine.playMove(Board::index(9, 9));
    engine.playMove(Board::index(9, 10));
    engine.playMove(Board::index(8, 8));

    AI ai(Cell::WHITE);

    {
        const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
        int move = ai.findBestMove(engine, 10);
        const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - t0).count();

        CHECK(move >= 0);
        CHECK(engine.isLegalMove(move));
        CHECK(ms <= 500);
    }

    {
        int move = ai.findBestMoveTimed(engine, std::chrono::milliseconds(500));
        CHECK(move >= 0);
        CHECK(engine.isLegalMove(move));
        CHECK(ai.lastStats().depthReached >= 10);
        CHECK(ai.lastStats().elapsedMs <= 500);
    }
}
