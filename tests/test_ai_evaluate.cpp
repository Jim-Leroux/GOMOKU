#include "test.hpp"
#include "AI.hpp"
#include "Board.hpp"

void test_ai_evaluate()
{
    std::cout << "-- AI::evaluate --\n";

    // Empty board: no shapes, no captures, dead-even score.
    {
        Board board;
        AI ai(Cell::BLACK);
        CHECK(ai.evaluate(board) == 0);
    }

    // A single stone isn't a "shape" of any scored length (2-5) yet.
    {
        Board board;
        board.applyMove(Board::index(9, 9), Cell::BLACK);
        AI ai(Cell::BLACK);
        CHECK(ai.evaluate(board) == 0);
    }

    // Two AI stones, open on both ends, is worth exactly the tuned constant
    // for an open pair.
    {
        Board board;
        board.applyMove(Board::index(9, 9), Cell::BLACK);
        board.applyMove(Board::index(9, 10), Cell::BLACK);
        AI ai(Cell::BLACK);
        CHECK(ai.evaluate(board) == 50); // shapeScore(2, openEnds=2)
    }

    // The exact same shape, but belonging to the opponent, must score the
    // mirror image: same magnitude, opposite sign.
    {
        Board board;
        board.applyMove(Board::index(9, 9), Cell::WHITE);
        board.applyMove(Board::index(9, 10), Cell::WHITE);
        AI ai(Cell::BLACK);
        CHECK(ai.evaluate(board) == -50); // -shapeScore(2, openEnds=2)
    }

    // An open three (both ends free) is worth far more than a three that's
    // blocked on one side — that gap is exactly what makes an open three an
    // unstoppable threat one move later.
    {
        Board openThree, blockedThree, deadThree;
        for (int col = 5; col <= 7; ++col)
        {
            openThree.applyMove(Board::index(9, col), Cell::BLACK);
            blockedThree.applyMove(Board::index(9, col), Cell::BLACK);
            deadThree.applyMove(Board::index(9, col), Cell::BLACK);
        }
        blockedThree.applyMove(Board::index(9, 4), Cell::WHITE);  // blocks one end
        deadThree.applyMove(Board::index(9, 4), Cell::WHITE);     // blocks both ends
        deadThree.applyMove(Board::index(9, 8), Cell::WHITE);

        AI ai(Cell::BLACK);
        int openScore    = ai.evaluate(openThree);
        int blockedScore = ai.evaluate(blockedThree);
        int deadScore    = ai.evaluate(deadThree);

        CHECK(openScore == 1000);    // shapeScore(3, openEnds=2)
        CHECK(blockedScore == 150);  // shapeScore(3, openEnds=1); the lone
                                      // blocking White stone scores 0 itself
                                      // (only runs of 2+ are worth anything)
        CHECK(deadScore == 0);       // shapeScore(3, openEnds=0)
    }

    // Captures use non-linear scoring (pairs² × 200): each extra pair is
    // worth more than the last, because captures are progress toward the
    // fixed win threshold of 5 pairs.
    {
        Board onePair, twoPairs;
        onePair.addCaptures(Cell::BLACK, 2);   // 1 pair  -> 1² × 200 = 200
        twoPairs.addCaptures(Cell::BLACK, 4);  // 2 pairs -> 2² × 200 = 800

        AI ai(Cell::BLACK);
        CHECK(ai.evaluate(onePair) == 200);
        CHECK(ai.evaluate(twoPairs) == 800);
        // 800 > 2 × 200 — doubling pairs more than doubles the score.
        CHECK(ai.evaluate(twoPairs) > 2 * ai.evaluate(onePair));

        AI aiWhite(Cell::WHITE);
        CHECK(aiWhite.evaluate(onePair) == -200);
    }
}
