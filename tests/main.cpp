#include "test.hpp"

void test_board();
void test_game_engine_basics();
void test_alignment();

int main()
{
    test_board();
    test_game_engine_basics();
    test_alignment();
    return Test::summary();
}
