#include "test.hpp"

void test_board();
void test_game_engine_basics();

int main()
{
    test_board();
    test_game_engine_basics();
    return Test::summary();
}
