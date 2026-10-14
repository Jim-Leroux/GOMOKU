import React, { useEffect, useState, useCallback } from 'react';
import './index.css';

const AI_SEARCH_DEPTH = 4;

const formatThinkingTime = (ms) => {
  if (ms == null) return '—';
  if (ms < 1000) return `${ms} ms`;
  return `${(ms / 1000).toFixed(1)} s`;
};

function App() {
  const [engine, setEngine] = useState(null);
  const [board, setBoard] = useState(Array(361).fill(0));
  const [loading, setLoading] = useState(true);
  const [mode, setMode] = useState(null);
  const [aiThinking, setAiThinking] = useState(false);
  const [thinkingTimeMs, setThinkingTimeMs] = useState(null);
  const [searchDepth, setSearchDepth] = useState(null);
  const [liveThinkingMs, setLiveThinkingMs] = useState(0);
  const [suggestedMove, setSuggestedMove] = useState(null);
  const [hintCalculating, setHintCalculating] = useState(false);
  const [illegalMoves, setIllegalMoves] = useState(new Set());
  const [showIllegalMoves, setShowIllegalMoves] = useState(true);
  const [darkMode, setDarkMode] = useState(() => {
    return localStorage.getItem('gomoku_theme') === 'dark';
  });

  useEffect(() => {
    if (darkMode) {
      document.documentElement.classList.add('dark-theme');
      localStorage.setItem('gomoku_theme', 'dark');
    } else {
      document.documentElement.classList.remove('dark-theme');
      localStorage.setItem('gomoku_theme', 'light');
    }
  }, [darkMode]);

  const updateBoard = (gameEngine) => {
    const state = gameEngine.getBoardState();
    const jsArray = [];
    for (let i = 0; i < state.size(); i++) {
      jsArray.push(state.get(i));
    }
    setBoard(jsArray);

    const illegals = new Set();
    if (gameEngine && !gameEngine.isTerminal()) {
      for (let i = 0; i < 361; i++) {
        if (jsArray[i] === 0) {
          const r = Math.floor(i / 19), c = i % 19;
          if (!gameEngine.isLegalMove(r, c)) {
            illegals.add(i);
          }
        }
      }
    }
    setIllegalMoves(illegals);
  };

  useEffect(() => {
    let checkInterval = setInterval(async () => {
      if (window.createGomoku) {
        clearInterval(checkInterval);
        try {
          const Module = await window.createGomoku();
          const gameInstance = new Module.WebGame();
          setEngine(gameInstance);
          updateBoard(gameInstance);
          setLoading(false);
        } catch (e) {
          console.error("Failed to load WASM engine", e);
        }
      }
    }, 100);
    return () => clearInterval(checkInterval);
  }, []);

  useEffect(() => {
    if (!aiThinking) return;
    const start = performance.now();
    const intervalId = setInterval(() => {
      setLiveThinkingMs(Math.round(performance.now() - start));
    }, 50);
    return () => clearInterval(intervalId);
  }, [aiThinking]);

  const scheduleAiMove = () => {
    if (!engine || engine.isTerminal())
      return;
    setAiThinking(true);
    setLiveThinkingMs(0);
    setTimeout(() => {
      try {
        const move = engine.findBestMoveTimedMs(500);
        if (move >= 0) {
          const row = Math.floor(move / 19), col = move % 19;
          if (engine.isLegalMove(row, col)) {
            engine.playMove(row, col);
            updateBoard(engine);
          }
        }
        setThinkingTimeMs(Number(engine.getLastAiThinkingTimeMs()));
        setSearchDepth(Number(engine.getLastAiSearchDepth()));
      } catch (e) {
        console.error('AI move failed', e);
      } finally {
        setAiThinking(false);
      }
    }, 0);
  };

  const handlePlay = (index) => {
    if (!engine || engine.isTerminal() || aiThinking)
      return;
    const row = Math.floor(index / 19);
    const col = index % 19;
    if (mode === 'pvai' && !engine.isHumanTurn())
      return;
    if (engine.isLegalMove(row, col)) {
      setSuggestedMove(null);
      engine.playMove(row, col);
      updateBoard(engine);
      if (mode === 'pvai')
        scheduleAiMove();
    }
  };

  const handleUndo = useCallback(() => {
    if (!engine || aiThinking) return;
    const count = engine.getMoveCount();
    if (count === 0) return;

    setSuggestedMove(null);
    if (mode === 'pvai') {
      // In PvAI, if it's the human's turn, the AI made the last move.
      // Undo both the AI move and the human move so the human can play again.
      engine.undoMove();
      if (engine.getMoveCount() > 0 && !engine.isHumanTurn()) {
        engine.undoMove();
      }
    } else {
      // In PvP, undo 1 move
      engine.undoMove();
    }
    updateBoard(engine);
    setThinkingTimeMs(null);
    setSearchDepth(null);
  }, [engine, aiThinking, mode]);

  const handleSuggestMove = useCallback(() => {
    if (!engine || engine.isTerminal() || aiThinking || hintCalculating)
      return;
    setHintCalculating(true);
    setTimeout(() => {
      try {
        const move = engine.getSuggestedMove ? engine.getSuggestedMove() : engine.findBestMoveTimedMs(500);
        if (move >= 0) {
          setSuggestedMove(move);
        }
      } catch (e) {
        console.error('Failed to get suggested move', e);
      } finally {
        setHintCalculating(false);
      }
    }, 10);
  }, [engine, aiThinking, hintCalculating]);

  useEffect(() => {
    const handleKeyDown = (e) => {
      if (mode === null || aiThinking) return;
      if (e.key === 'u' || e.key === 'U') {
        handleUndo();
      } else if (e.key === 'h' || e.key === 'H') {
        handleSuggestMove();
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [mode, aiThinking, handleUndo, handleSuggestMove]);

  const startGame = (chosenMode) => {
    engine.setMode(chosenMode === 'pvai' ? 1 : 0);
    engine.reset();
    updateBoard(engine);
    setThinkingTimeMs(null);
    setSearchDepth(null);
    setAiThinking(false);
    setSuggestedMove(null);
    setHintCalculating(false);
    setMode(chosenMode);
  };
  
  const backToMenu = () => {
    engine.reset();
    updateBoard(engine);
    setThinkingTimeMs(null);
    setSearchDepth(null);
    setAiThinking(false);
    setSuggestedMove(null);
    setHintCalculating(false);
    setMode(null);
  };

  if (loading) {
    return (
      <div className="loading-screen">
        <div className="spinner"></div>
        <h2>Chargement de l'IA (C++)...</h2>
      </div>
    );
  }

  if (!loading && mode === null) {
    return (
      <div className="app-container">
        <div className="menu-screen glass-panel">
          <div className="menu-header">
            <h1 className="title">Gomoku Master</h1>
            <button
              className="theme-toggle-btn"
              onClick={() => setDarkMode(!darkMode)}
              title={darkMode ? "Passer en mode clair" : "Passer en mode sombre"}
            >
              {darkMode ? '☀️ Mode Clair' : '🌙 Mode Sombre'}
            </button>
          </div>
          <button className="btn-primary" onClick={() => startGame('pvp')}>
            Joueur vs Joueur
          </button>
          <button className="btn-primary" onClick={() => startGame('pvai')}>
            Joueur vs IA
          </button>
        </div>
      </div>
    );
  }

  const winner = engine.getWinner();

  return (
    <div className="app-container">
      <header className="glass-panel header">
        <div className="header-brand">
          <h1 className="title">Gomoku Master</h1>
          <button
            className="theme-toggle-icon-btn"
            onClick={() => setDarkMode(!darkMode)}
            title={darkMode ? "Passer en mode clair" : "Passer en mode sombre"}
          >
            {darkMode ? '☀️' : '🌙'}
          </button>
        </div>
        <div className="stats">
          <div className="stat-box">
            <span className="label">Tour</span>
            <div className={`turn-indicator ${engine.getCurrentPlayer() === 1 ? 'black' : 'white'}`}></div>
          </div>
          <div className="stat-box">
            <span className="label">Captures Noires</span>
            <span className="value">{engine.getBlackCaptures()}</span>
          </div>
          <div className="stat-box">
            <span className="label">Captures Blanches</span>
            <span className="value">{engine.getWhiteCaptures()}</span>
          </div>
          {mode === 'pvai' && (
            <>
              <div className="stat-box">
                <span className="label">Profondeur</span>
                <span className={`value ai-stat-value ${aiThinking ? 'active' : ''}`}>
                  {aiThinking ? AI_SEARCH_DEPTH : searchDepth ?? '—'}
                </span>
              </div>
              <div className="stat-box">
                <span className="label">Réflexion IA</span>
                <span className={`value ai-stat-value ${aiThinking ? 'active' : ''}`}>
                  {formatThinkingTime(aiThinking ? liveThinkingMs : thinkingTimeMs)}
                </span>
              </div>
            </>
          )}
        </div>
      </header>

      <div className="controls-bar">
        <button
          className="btn-secondary"
          onClick={handleSuggestMove}
          disabled={!engine || aiThinking || hintCalculating || engine.isTerminal()}
          title="Afficher la case du meilleur coup (Raccourci: H)"
        >
          <span className="icon">💡</span> {hintCalculating ? 'Calcul...' : 'Coup suggéré (H)'}
        </button>
        <button
          className="btn-secondary"
          onClick={handleUndo}
          disabled={!engine || aiThinking || hintCalculating || engine.getMoveCount() === 0}
          title="Annuler le coup (Raccourci: U)"
        >
          <span className="icon">↩</span> Annuler coup (U)
        </button>
        <button
          className={`btn-secondary ${showIllegalMoves ? 'active-toggle' : ''}`}
          onClick={() => setShowIllegalMoves(!showIllegalMoves)}
          title="Afficher/Masquer les coups interdits (Double-trois)"
        >
          <span className="icon">{showIllegalMoves ? '🚫' : '⭕'}</span> Coups interdits {showIllegalMoves ? 'ON' : 'OFF'}
        </button>
        <button
          className="btn-secondary"
          onClick={backToMenu}
          disabled={aiThinking || hintCalculating}
          title="Retourner au menu principal"
        >
          <span className="icon">⌂</span> Menu principal
        </button>
      </div>

      {winner !== 0 && (
        <div className="winner-banner glass-panel">
          <h2>{winner === 1 ? 'Les Noirs' : 'Les Blancs'} ont gagné !</h2>
          <div className="winner-actions">
            <button className="btn-secondary" onClick={handleUndo}>
              <span className="icon">↩</span> Annuler dernier coup
            </button>
            <button className="btn-primary" onClick={backToMenu}>Rejouer</button>
          </div>
        </div>
      )}

      <main className="board-container">
        <div className="board">
          {board.map((cell, index) => {
            const isBlack = cell === 1;
            const isWhite = cell === 2;
            const isSuggested = suggestedMove === index && cell === 0;
            const isIllegal = showIllegalMoves && cell === 0 && illegalMoves.has(index);
            const turnColor = engine.getCurrentPlayer() === 1 ? 'black' : 'white';
            return (
              <div
                key={index}
                className={`cell ${isIllegal ? 'illegal-cell' : ''}`}
                onClick={() => handlePlay(index)}
                title={isIllegal ? 'Coup interdit (Double-trois)' : undefined}
              >
                <div className="crosshair"></div>
                {(isBlack || isWhite) && (
                  <div className={`stone ${isBlack ? 'black' : 'white'} drop-in`}></div>
                )}
                {isSuggested && (
                  <div className={`suggested-indicator ${turnColor}`}>
                    <div className="suggested-pulse"></div>
                  </div>
                )}
                {isIllegal && (
                  <div className="illegal-marker">
                    <div className="illegal-icon">✕</div>
                  </div>
                )}
              </div>
            );
          })}
        </div>
      </main>
    </div>
  );
}

export default App;
