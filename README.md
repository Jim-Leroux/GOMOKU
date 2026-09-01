# Gomoku (React & WebAssembly)

Implémentation du jeu de Gomoku avec un **moteur haute-performance en C++** et une **interface visuelle moderne en React**.

---

## 🚀 Lancer le projet

L'ensemble de la compilation (C++ vers WebAssembly) est automatisé via Docker. Vous n'avez pas besoin d'installer le SDK Emscripten localement.

| Commande | Action |
|---|---|
| `make run-web` | Compile le C++ en `.wasm`, copie les fichiers vers React, et lance le serveur de développement Vite (avec Hot-Reload). |
| `make run-prod` | Crée l'architecture de production multi-étapes via `docker-compose` et héberge le jeu optimisé sur le port `8080` avec Nginx. |
| `make re` | Nettoie tous les fichiers générés (`fclean`) et relance une compilation propre (`run-web`). |

---

## 🧠 Guide pour l'équipe (C++)

> [!WARNING]  
> **Impact de l'intégration React :**  
> L'interface native **SFML** et les cibles de compilation de bureau ont été supprimées du `Makefile`. Vous testerez désormais votre algorithme de jeu directement dans le navigateur !

### Ce que vous devez savoir :
1. **Indépendance totale :** Vos fichiers cœurs (`Board.cpp`, `GameEngine.cpp`) doivent rester en **C++17 pur**. Le moteur ne doit avoir aucune dépendance au web.
2. **Le pont WasmBindings :** L'interface React communique avec votre C++ via le fichier `src/WasmBindings.cpp`. Si vous ajoutez une méthode utile à l'interface (ex: `getWinningStones()`), vous devez **obligatoirement l'enregistrer** dans la macro `EMSCRIPTEN_BINDINGS` de ce fichier.
3. **Performance :** L'exécution du `.wasm` dans le navigateur est quasi-aussi rapide que le natif. N'hésitez pas à implémenter des IA (Minimax, Monte-Carlo) poussées.
4. **💡 Astuce de Debug (std::cout) :** Si vous utilisez `std::cout` ou `printf` dans votre code C++, le texte s'affichera directement dans la **Console Développeur (F12) de votre navigateur Web**. Parfait pour débugger l'algorithme !

---

## 🎨 Guide pour l'équipe (Front-end)

L'interface se trouve dans le dossier `web/`.
- Propulsé par **Vite** pour un démarrage instantané.
- Zéro framework lourd (pas de Tailwind) : design 100% **Vanilla CSS** (`index.css`) inspiré du thème **Monokai Pro Light**.
- Logique asynchrone dans `App.jsx` pour attendre le chargement du moteur WebAssembly avant d'afficher le plateau.

---

## Architecture du Moteur C++

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

#### API de `Board`

```cpp
// Conversion coordonnées ↔ index plat
static int  Board::index(int row, int col);
static int  Board::row(int idx);
static int  Board::col(int idx);
static bool Board::isValid(int row, int col);

// Lecture des cellules
Cell get(int idx) const;
Cell get(int row, int col) const;

// Historique
void applyMove(int pos, Cell player, const std::vector<int>& captured = {});
void undoMove();
int  moveCount() const;

// Captures
int getCaptureCount(Cell player) const;
```

---

### `GameEngine` — le moteur de règles

**Fichiers** : `include/GameEngine.hpp` · `src/GameEngine.cpp`

Gère le déroulement de la partie : alternance des tours, validation des coups, captures, et détection de fin de partie.

#### API de `GameEngine`

```cpp
// Validation
bool isLegalMove(int pos) const;

// Jouer
void playMove(int pos);
void undoMove();

// État
bool isTerminal()       const;  // partie terminée ?
Cell getWinner()        const;  // BLACK, WHITE, ou EMPTY
Cell getCurrentPlayer() const;  // à qui de jouer
int  getCaptureCount(Cell player) const;

// Réinitialisation
void reset();
```

---

## Règles du jeu

### Règles implémentées
- Plateau 19×19, tableau 1D
- Alternance des tours
- Historique et annulation (`undoMove`)
- Compteurs de captures

### Règles à implémenter
- Détection alignement 5+ pierres (4 directions)
- Capture d'une paire flanquée
- Règle "on ne joue pas dans une capture"
- Victoire à 10 pierres capturées
- Interdiction du double-three (avec exception par capture)
- Endgame Capture
