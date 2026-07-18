#include "test.hpp"
#include "Board.hpp"

void test_board()
{
    std::cout << "-- Board --\n";

    // Coordinate helpers
    CHECK(Board::index(0, 0) == 0);
    CHECK(Board::index(9, 9) == 9 * BOARD_SIZE + 9);
    CHECK(Board::row(Board::index(3, 5)) == 3);
    CHECK(Board::col(Board::index(3, 5)) == 5);
    CHECK(Board::isValid(0, 0));
    CHECK(Board::isValid(18, 18));
    CHECK(!Board::isValid(19, 0));
    CHECK(!Board::isValid(0, -1));
    CHECK(Board::isValidIdx(0));
    CHECK(Board::isValidIdx(360));
    CHECK(!Board::isValidIdx(361));

    // A fresh board is empty everywhere
    Board board;
    CHECK(board.isEmpty(Board::index(9, 9)));
    CHECK(board.get(9, 9) == Cell::EMPTY);
    CHECK(board.moveCount() == 0);

    // applyMove places a stone and records history
    board.applyMove(Board::index(9, 9), Cell::BLACK);
    CHECK(board.get(9, 9) == Cell::BLACK);
    CHECK(!board.isEmpty(Board::index(9, 9)));
    CHECK(board.moveCount() == 1);

    // undoMove restores the previous state exactly
    board.undoMove();
    CHECK(board.isEmpty(Board::index(9, 9)));
    CHECK(board.moveCount() == 0);

    // applyMove can remove captured stones and credit the capturer
    board.applyMove(Board::index(5, 5), Cell::WHITE);
    board.applyMove(Board::index(5, 6), Cell::WHITE);
    board.applyMove(Board::index(5, 7), Cell::BLACK,
                    { Board::index(5, 5), Board::index(5, 6) });
    CHECK(board.isEmpty(Board::index(5, 5)));
    CHECK(board.isEmpty(Board::index(5, 6)));
    CHECK(board.getCaptureCount(Cell::BLACK) == 2);

    // undoMove reverses a capturing move too: stones come back, counter reverts
    board.undoMove();
    CHECK(board.get(5, 5) == Cell::WHITE);
    CHECK(board.get(5, 6) == Cell::WHITE);
    CHECK(board.getCaptureCount(Cell::BLACK) == 0);
}
