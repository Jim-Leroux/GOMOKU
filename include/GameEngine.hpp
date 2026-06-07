#pragma once

#include "Board.hpp"

// ─────────────────────────────────────────────────────────────────────────────
//  GameEngine — Moteur de règles du Gomoku
//
//  Gère le déroulement de la partie : alternance des tours, validation des
//  coups, captures, détection de fin de partie.
//  L'interface graphique appelle cette classe pour jouer et interroger l'état.
// ─────────────────────────────────────────────────────────────────────────────

class GameEngine
{
public:
    GameEngine();

    // ── Accès au plateau (lecture seule pour l'affichage) ───────────────────
    const Board& getBoard() const;

    // ── Validation & jeu ────────────────────────────────────────────────────
    // Retourne true si la position est jouable (case vide + toutes les règles).
    bool isLegalMove(int pos) const;

    // Joue un coup pour le joueur courant : capture, changement de tour,
    // détection de fin de partie. Lance une exception si le coup est illégal.
    void playMove(int pos);

    // Annule le dernier coup joué (utile pour une éventuelle fonctionnalité undo).
    void undoMove();

    // ── État de la partie ───────────────────────────────────────────────────
    bool isTerminal()        const;
    Cell getWinner()         const;
    Cell getCurrentPlayer()  const;
    int  getMoveCount()      const;

    // ── Captures ────────────────────────────────────────────────────────────
    int getCaptureCount(Cell player) const;

    // ── Utilitaire ──────────────────────────────────────────────────────────
    void reset();

private:
    Board m_board;
    Cell  m_currentPlayer;
    Cell  m_winner;
    bool  m_terminal;

    // ── Helpers internes (implémentés dans les tâches suivantes) ────────────
    std::vector<int> computeCaptures(int pos, Cell player) const;
    bool             checkAlignment(int pos, Cell player)  const;
    bool             checkDoubleThree(int pos, Cell player) const;
    bool             checkEndgameCapture(Cell player)       const;
};
