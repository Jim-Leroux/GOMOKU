#include <emscripten/bind.h>
#include "GameEngine.hpp"
#include <vector>

using namespace emscripten;

// ─────────────────────────────────────────────────────────────────────────────
// WebGame — Wrapper pour exposer proprement le C++ à JavaScript via Emscripten
// ─────────────────────────────────────────────────────────────────────────────
class WebGame {
public:
    WebGame() : engine() {}

    // Vérifie si un coup est jouable aux coordonnées (row, col)
    bool isLegalMove(int row, int col) const {
        if (!Board::isValid(row, col)) return false;
        return engine.isLegalMove(Board::index(row, col));
    }

    // Joue un coup
    void playMove(int row, int col) {
        if (!Board::isValid(row, col)) return;
        engine.playMove(Board::index(row, col));
    }

    // Récupère le plateau sous forme de tableau à 1 dimension (taille 361)
    // En JS, vous recevrez un Array d'entiers (0=Vide, 1=Noir, 2=Blanc)
    std::vector<int> getBoardState() const {
        std::vector<int> state;
        const Board& b = engine.getBoard();
        state.reserve(361);
        for (int i = 0; i < 361; ++i) {
            state.push_back(static_cast<int>(b.get(i)));
        }
        return state;
    }

    int getCurrentPlayer() const {
        return static_cast<int>(engine.getCurrentPlayer());
    }

    int getWinner() const {
        return static_cast<int>(engine.getWinner());
    }

    bool isTerminal() const {
        return engine.isTerminal();
    }

    int getMoveCount() const {
        return engine.getMoveCount();
    }
    
    int getBlackCaptures() const {
        return engine.getCaptureCount(Cell::BLACK);
    }
    
    int getWhiteCaptures() const {
        return engine.getCaptureCount(Cell::WHITE);
    }

    void undoMove() {
        engine.undoMove();
    }

    void reset() {
        engine.reset();
    }

private:
    GameEngine engine;
};

// ─────────────────────────────────────────────────────────────────────────────
// Déclaration des Bindings Emscripten
// C'est ici qu'on définit le "Contrat" qui sera visible dans votre React
// ─────────────────────────────────────────────────────────────────────────────
EMSCRIPTEN_BINDINGS(gomoku_module) {
    // Permet au JS de comprendre et de manipuler le std::vector<int> renvoyé par getBoardState
    register_vector<int>("VectorInt");

    // On expose la classe WebGame sous le même nom en JS
    class_<WebGame>("WebGame")
        .constructor<>()
        .function("isLegalMove", &WebGame::isLegalMove)
        .function("playMove", &WebGame::playMove)
        .function("getBoardState", &WebGame::getBoardState)
        .function("getCurrentPlayer", &WebGame::getCurrentPlayer)
        .function("getWinner", &WebGame::getWinner)
        .function("isTerminal", &WebGame::isTerminal)
        .function("getMoveCount", &WebGame::getMoveCount)
        .function("getBlackCaptures", &WebGame::getBlackCaptures)
        .function("getWhiteCaptures", &WebGame::getWhiteCaptures)
        .function("undoMove", &WebGame::undoMove)
        .function("reset", &WebGame::reset);
}
