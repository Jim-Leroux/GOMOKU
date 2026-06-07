# Gomoku

Implémentation du jeu de Gomoku en C++ avec SFML.

---

## Compilation

### Prérequis — SFML

| OS | Commande |
|---|---|
| macOS | `brew install sfml` |
| Ubuntu / Debian | `sudo apt install libsfml-dev` |
| Arch Linux | `sudo pacman -S sfml` |

### Règles Makefile

```bash
make        # Compile (sans re-link si rien n'a changé)
make re     # Recompile tout depuis zéro
make clean  # Supprime les fichiers objets
make fclean # Supprime les objets + l'exécutable Gomoku
```

---

## Structure du projet

```
gomoku/
├── Makefile
├── README.md
├── include/
│   ├── Board.hpp        # Plateau et structure de données
│   └── GameEngine.hpp   # Moteur de règles
└── src/
    ├── Board.cpp
    ├── GameEngine.cpp
    └── main.cpp
```

---

## Architecture

### `Board` — le plateau

**Fichiers** : `include/Board.hpp` · `src/Board.cpp`

Le plateau 19×19 est stocké comme un **tableau 1D** de 361 cellules (`std::array<Cell, 361>`).

**Pourquoi un tableau 1D plutôt que 2D ?**
- Les cellules sont contiguës en mémoire → meilleure utilisation du cache CPU
- L'accès à une case est `cells[row * 19 + col]`, une simple multiplication entière
- La copie du plateau entier est une `memcpy` de 361 octets

**L'enum `Cell` est encodé sur `int8_t`** (1 octet par case) : le plateau tient en 361 octets contre 1 444 octets avec `int`.

```cpp
enum class Cell : int8_t { EMPTY = 0, BLACK = 1, WHITE = 2 };
```

**L'historique des coups** (struct `Move`) permet un `undoMove()` instantané : chaque coup enregistre sa position, le joueur et la liste des pierres capturées. L'annulation restaure exactement l'état précédent sans recopier le plateau.

#### API de `Board`

```cpp
// Conversion coordonnées ↔ index plat
static int  Board::index(int row, int col);   // (9, 9)  → 180
static int  Board::row(int idx);              //  180    → 9
static int  Board::col(int idx);              //  180    → 9
static bool Board::isValid(int row, int col); // in-bounds check
static bool Board::isValidIdx(int idx);

// Lecture des cellules
Cell get(int idx) const;
Cell get(int row, int col) const;
bool isEmpty(int idx) const;

// Historique
void applyMove(int pos, Cell player, const std::vector<int>& captured = {});
void undoMove();
int  moveCount() const;
const std::vector<Move>& history() const;

// Captures
int getCaptureCount(Cell player) const;

// Utilitaire
void reset();
bool isFull() const;
void print() const;  // affichage ASCII pour le debug
```

---

### `GameEngine` — le moteur de règles

**Fichiers** : `include/GameEngine.hpp` · `src/GameEngine.cpp`

Gère le déroulement de la partie : alternance des tours, validation des coups selon les règles, captures, et détection de fin de partie.

L'interface graphique (SFML) passe par `GameEngine` pour tout ce qui concerne le jeu.

#### API de `GameEngine`

```cpp
// Accès au plateau (lecture seule pour l'affichage)
const Board& getBoard() const;

// Validation
bool isLegalMove(int pos) const;  // case vide + toutes les règles actives

// Jouer
void playMove(int pos);   // joue pour le joueur courant, effectue les captures
void undoMove();          // annule le dernier coup

// État
bool isTerminal()       const;  // partie terminée ?
Cell getWinner()        const;  // BLACK, WHITE, ou EMPTY si pas encore fini
Cell getCurrentPlayer() const;  // à qui de jouer
int  getMoveCount()     const;  // nombre de coups joués

// Captures
int getCaptureCount(Cell player) const;

// Réinitialisation
void reset();
```

---

## Règles du jeu

### Règles implementées

| Règle | Statut |
|---|---|
| Plateau 19×19, tableau 1D | ✅ |
| Alternance des tours | ✅ |
| Historique et annulation (`undoMove`) | ✅ |
| Compteurs de captures | ✅ |

### Règles à implémenter

| Règle | Tâche |
|---|---|
| Détection alignement 5+ pierres (4 directions) | 2.1 |
| Capture d'une paire flanquée | 2.2 |
| Règle "on ne joue pas dans une capture" | 2.2 |
| Victoire à 10 pierres capturées | 2.2 |
| Interdiction du double-three | 2.3 |
| Exception double-three par capture | 2.3 |
| Endgame Capture | 2.4 |

---

## Règles Gomoku (rappel sujet)

- **Victoire par alignement** : 5 pierres ou plus dans n'importe quelle direction.
- **Capture** : flanquer une paire exacte de pierres adverses les retire du plateau.
- **Victoire par captures** : 10 pierres adverses capturées = victoire.
- **Double-three interdit** : on ne peut pas jouer un coup qui crée deux alignements de trois pierres libres simultanément.
  - Exception : si ce double-three est créé par une capture, le coup reste légal.
- **Endgame Capture** : un alignement de 5 ne gagne pas immédiatement si l'adversaire peut briser la ligne en capturant une paire.
# GOMOKU
