#include <iostream>
#include "GameEngine.hpp"

int main()
{
    GameEngine engine;

    std::cout << "=== Gomoku — Test moteur de jeu ===\n\n";

    std::cout << "Joueur courant : "
              << (engine.getCurrentPlayer() == Cell::BLACK ? "BLACK" : "WHITE") << "\n";
    std::cout << "Partie terminée : " << std::boolalpha << engine.isTerminal() << "\n\n";

    // Jouer quelques coups
    engine.playMove(Board::index(9, 9));
    engine.playMove(Board::index(9, 10));
    engine.playMove(Board::index(8, 9));

    std::cout << "Après 3 coups :\n";
    engine.getBoard().print();

    std::cout << "\nNombre de coups : " << engine.getMoveCount() << "\n";
    std::cout << "Joueur courant : "
              << (engine.getCurrentPlayer() == Cell::BLACK ? "BLACK" : "WHITE") << "\n\n";

    // Undo
    engine.undoMove();
    std::cout << "Après undoMove :\n";
    std::cout << "Nombre de coups : " << engine.getMoveCount() << "\n";
    std::cout << "Joueur courant : "
              << (engine.getCurrentPlayer() == Cell::BLACK ? "BLACK" : "WHITE") << "\n";

    return 0;
}
