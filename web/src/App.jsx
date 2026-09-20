import React, { useEffect, useState } from 'react';
import './index.css';

function App() {
  const [engine, setEngine] = useState(null);
  const [board, setBoard] = useState(Array(361).fill(0));
  const [loading, setLoading] = useState(true);
  const [mode, setMode] = useState(null);
  const [aiThinking, setAiThinking] = useState(false);  

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

  const scheduleAiMove = () => {
    setAiThinking(true);
    setTimeout(() => {
      const move = engine.findBestMoveAtDepth(4);
      if (move >= 0) {
        const row = Math.floor(move / 19), col = move % 19;
        engine.playMove(row, col);
        updateBoard(engine);
      }
      setAiThinking(false);
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
    }
    if (mode === 'pvai')
      scheduleAiMove();
  };

  const startGame = (chosenMode) => {
    engine.setMode(chosenMode === 'pvai' ? 1 : 0);
    engine.reset();
    updateBoard(engine);
    setMode(chosenMode);
  };
  
  const backToMenu = () => {
    engine.reset();
    updateBoard(engine);
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
          {aiThinking && <span className="thinking-label">L'IA réfléchit…</span>}
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
