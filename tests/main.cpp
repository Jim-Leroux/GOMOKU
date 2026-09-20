#include "test.hpp"

void test_board();
void test_game_engine_basics();
void test_alignment();
void test_capture();
void test_double_three();
void test_endgame_capture();
void test_ai_evaluate();
void test_ai_search();
void test_ai_async();

int main()
{
    test_board();
    test_game_engine_basics();
    test_alignment();
    test_capture();
    test_double_three();
    test_endgame_capture();
    test_ai_evaluate();
    test_ai_search();
    test_ai_async();
    return Test::summary();
}
