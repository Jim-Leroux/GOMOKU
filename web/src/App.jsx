import React, { useEffect, useState } from 'react';
import './index.css';

function App() {
  const [engine, setEngine] = useState(null);
  const [board, setBoard] = useState(Array(361).fill(0));
  const [loading, setLoading] = useState(true);

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

  const handlePlay = (index) => {
    if (!engine || engine.isTerminal()) return;
    const row = Math.floor(index / 19);
    const col = index % 19;
    
    if (engine.isLegalMove(row, col)) {
      engine.playMove(row, col);
      updateBoard(engine);
    }
  };

  if (loading) {
    return (
      <div className="loading-screen">
        <div className="spinner"></div>
        <h2>Chargement de l'IA (C++)...</h2>
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
        </div>
      </header>

      {winner !== 0 && (
        <div className="winner-banner glass-panel">
          <h2>{winner === 1 ? 'Les Noirs' : 'Les Blancs'} ont gagné !</h2>
          <button className="btn-primary" onClick={() => { engine.reset(); updateBoard(engine); }}>Rejouer</button>
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
