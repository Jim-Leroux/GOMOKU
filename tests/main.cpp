#include "test.hpp"

void test_board();
void test_game_engine_basics();
void test_alignment();
void test_capture();
void test_double_three();

int main()
{
    test_board();
    test_game_engine_basics();
    test_alignment();
    test_capture();
    test_double_three();
    return Test::summary();
}
