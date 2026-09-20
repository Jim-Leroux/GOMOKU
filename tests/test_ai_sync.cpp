#include "test.hpp"
#include "AI.hpp"
#include <future>

void test_ai_async()
{
    std::cout << "-- AI async search pattern (web GUI model) --\n";

    GameEngine engine;
    engine.playMove(Board::index(9, 9));
    engine.playMove(Board::index(9, 10));
    engine.playMove(Board::index(8, 8));

    GameEngine snapshot = engine;
    AI ai(Cell::WHITE);

    std::future<int> future = std::async(std::launch::async, [snapshot, ai]() mutable {
        return ai.findBestMove(snapshot, 3);
    });

    const int move = future.get();
    CHECK(move >= 0);
    CHECK(engine.isLegalMove(move));

    engine.playMove(move);
    CHECK(engine.getMoveCount() == 4);
}
