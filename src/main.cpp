#include "GameEngine.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

namespace
{
    constexpr int   WINDOW_W     = 900;
    constexpr int   WINDOW_H     = 980;
    constexpr int   BOARD_MARGIN = 50;
    constexpr int   INFO_H       = 80;
    constexpr float STONE_RADIUS = 14.f;

    int nearestIntersection(const sf::Vector2f& click, float cellSize)
    {
        float fx = (click.x - BOARD_MARGIN) / cellSize;
        float fy = (click.y - BOARD_MARGIN - INFO_H) / cellSize;
        int   col = static_cast<int>(std::round(fx));
        int   row = static_cast<int>(std::round(fy));
        if (!Board::isValid(row, col))
            return -1;
        return Board::index(row, col);
    }

    sf::Vector2f intersectionPixel(int pos, float cellSize)
    {
        int r = Board::row(pos);
        int c = Board::col(pos);
        return {BOARD_MARGIN + c * cellSize,
                BOARD_MARGIN + INFO_H + r * cellSize};
    }

    std::string cellName(Cell c)
    {
        if (c == Cell::BLACK)
            return "Black";
        if (c == Cell::WHITE)
            return "White";
        return "-";
    }

    std::string winReasonText(WinReason r)
    {
        if (r == WinReason::ALIGNMENT)
            return "5 in a row";
        if (r == WinReason::CAPTURE)
            return "10 captures";
        return "";
    }

    bool loadUIFont(sf::Font& font)
    {
        static const char* paths[] = {
            "/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf",
            "/usr/share/fonts/adwaita-sans-fonts/AdwaitaSans-Regular.ttf",
            "/usr/share/fonts/open-sans/OpenSans-Regular.ttf",
            "/usr/share/fonts/google-droid-sans-fonts/DroidSans.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
        };

        for (const char* path : paths)
        {
            if (font.loadFromFile(path))
                return true;
        }
        return false;
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "Gomoku");
    window.setFramerateLimit(60);

    GameEngine engine;
    engine.setMode(GameMode::HUMAN_VS_AI);

    const float cellSize =
        static_cast<float>(std::min(WINDOW_W, WINDOW_H - INFO_H) - 2 * BOARD_MARGIN) /
        static_cast<float>(BOARD_SIZE - 1);

    sf::Font font;
    if (!loadUIFont(font))
    {
        std::cerr << "Gomoku: no UI font found (install liberation-sans-fonts)\n";
        return 1;
    }

    bool hotseat   = false;
    bool aiPending = false;

    while (window.isOpen())
    {
        if (aiPending && !hotseat && !engine.isTerminal())
        {
            engine.playAiMove();
            aiPending = false;
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::R)
                {
                    engine.reset();
                    aiPending = false;
                }
                if (event.key.code == sf::Keyboard::M)
                {
                    hotseat = !hotseat;
                    engine.setMode(hotseat ? GameMode::HOTSEAT : GameMode::HUMAN_VS_AI);
                    engine.reset();
                    aiPending = false;
                }
                if (event.key.code == sf::Keyboard::U)
                {
                    engine.undoMove();
                    aiPending = false;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                if (engine.isTerminal())
                    continue;

                if (!hotseat && (engine.getCurrentPlayer() != Cell::BLACK || aiPending))
                    continue;

                sf::Vector2f click(static_cast<float>(event.mouseButton.x),
                                   static_cast<float>(event.mouseButton.y));
                int pos = nearestIntersection(click, cellSize);
                if (pos >= 0 && engine.isLegalMove(pos))
                {
                    engine.playMove(pos);
                    if (!hotseat && !engine.isTerminal())
                        aiPending = true;
                }
            }
        }

        window.clear(sf::Color(220, 179, 92));

        sf::RectangleShape infoBar(sf::Vector2f(static_cast<float>(WINDOW_W), static_cast<float>(INFO_H)));
        infoBar.setFillColor(sf::Color(50, 50, 50));
        window.draw(infoBar);

        std::ostringstream info;
        info << "Turn: " << cellName(engine.getCurrentPlayer())
             << "  |  B caps: " << engine.getCaptureCount(Cell::BLACK)
             << "  W caps: " << engine.getCaptureCount(Cell::WHITE)
             << "  |  AI: " << engine.getLastAiTimeMs() << " ms"
             << "  depth " << engine.getLastSearchDepth()
             << "  |  [M] mode  [R] reset  [U] undo";

        if (engine.isTerminal())
        {
            if (engine.getWinner() != Cell::EMPTY)
                info << "  |  Winner: " << cellName(engine.getWinner())
                     << " (" << winReasonText(engine.getWinReason()) << ")";
            else
                info << "  |  Draw";
        }

        if (hotseat)
            info << "  |  Hotseat";

        sf::Text hud(info.str(), font, 16);
        hud.setFillColor(sf::Color::White);
        hud.setPosition(12.f, 28.f);
        window.draw(hud);

        for (int i = 0; i < BOARD_SIZE; ++i)
        {
            float x = BOARD_MARGIN + i * cellSize;
            float y0 = BOARD_MARGIN + INFO_H;
            float y1 = BOARD_MARGIN + INFO_H + (BOARD_SIZE - 1) * cellSize;

            sf::Vertex vLine[] = {
                {sf::Vector2f(x, y0), sf::Color::Black},
                {sf::Vector2f(x, y1), sf::Color::Black}};
            window.draw(vLine, 2, sf::Lines);

            float y = BOARD_MARGIN + INFO_H + i * cellSize;
            float x0 = BOARD_MARGIN;
            float x1 = BOARD_MARGIN + (BOARD_SIZE - 1) * cellSize;

            sf::Vertex hLine[] = {
                {sf::Vector2f(x0, y), sf::Color::Black},
                {sf::Vector2f(x1, y), sf::Color::Black}};
            window.draw(hLine, 2, sf::Lines);
        }

        const Board& board = engine.getBoard();
        for (int i = 0; i < BOARD_CELLS; ++i)
        {
            Cell cell = board.get(i);
            if (cell == Cell::EMPTY)
                continue;

            sf::Vector2f center = intersectionPixel(i, cellSize);
            sf::CircleShape stone(STONE_RADIUS);
            stone.setOrigin(STONE_RADIUS, STONE_RADIUS);
            stone.setPosition(center);
            stone.setFillColor(cell == Cell::BLACK ? sf::Color::Black : sf::Color::White);
            stone.setOutlineThickness(1.f);
            stone.setOutlineColor(sf::Color(30, 30, 30));
            window.draw(stone);
        }

        int last = engine.getLastMove();
        if (last >= 0)
        {
            sf::Vector2f center = intersectionPixel(last, cellSize);
            sf::CircleShape mark(5.f);
            mark.setOrigin(5.f, 5.f);
            mark.setPosition(center);
            mark.setFillColor(board.get(last) == Cell::BLACK ? sf::Color::Red : sf::Color::Red);
            window.draw(mark);
        }

        int suggested = engine.getSuggestedMove();
        if (hotseat && suggested >= 0 && board.isEmpty(suggested))
        {
            sf::Vector2f center = intersectionPixel(suggested, cellSize);
            sf::CircleShape ghost(STONE_RADIUS);
            ghost.setOrigin(STONE_RADIUS, STONE_RADIUS);
            ghost.setPosition(center);
            ghost.setFillColor(sf::Color(100, 100, 255, 100));
            ghost.setOutlineThickness(2.f);
            ghost.setOutlineColor(sf::Color(50, 50, 200, 180));
            window.draw(ghost);
        }

        window.display();
    }

    return 0;
}
