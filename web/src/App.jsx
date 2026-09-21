import React, { useEffect, useState } from 'react';
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

  const updateBoard = (gameEngine) => {
    const state = gameEngine.getBoardState();
    const jsArray = [];
    for (let i = 0; i < state.size(); i++) {
      jsArray.push(state.get(i));
    }
    setBoard(jsArray);
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
        const move = engine.findBestMoveAtDepth(AI_SEARCH_DEPTH);
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
      engine.playMove(row, col);
      updateBoard(engine);
      if (mode === 'pvai')
        scheduleAiMove();
    }
  };

  const startGame = (chosenMode) => {
    engine.setMode(chosenMode === 'pvai' ? 1 : 0);
    engine.reset();
    updateBoard(engine);
    setThinkingTimeMs(null);
    setSearchDepth(null);
    setAiThinking(false);
    setMode(chosenMode);
  };
  
  const backToMenu = () => {
    engine.reset();
    updateBoard(engine);
    setThinkingTimeMs(null);
    setSearchDepth(null);
    setAiThinking(false);
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
          <h1 className="title">Gomoku Master</h1>
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
        <h1 className="title">Gomoku Master</h1>
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

      {winner !== 0 && (
        <div className="winner-banner glass-panel">
          <h2>{winner === 1 ? 'Les Noirs' : 'Les Blancs'} ont gagné !</h2>
          <button className="btn-primary" onClick={backToMenu}>Rejouer</button>
        </div>
      )}

      <main className="board-container">
        <div className="board">
          {board.map((cell, index) => {
            const isBlack = cell === 1;
            const isWhite = cell === 2;
            return (
              <div key={index} className="cell" onClick={() => handlePlay(index)}>
                <div className="crosshair"></div>
                {(isBlack || isWhite) && (
                  <div className={`stone ${isBlack ? 'black' : 'white'} drop-in`}></div>
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
