#pragma once
#include <Arduino.h>

class MinesweeperGame {
public:
    static const int COLS  = 9;
    static const int ROWS  = 9;
    static const int MINES = 10;

    enum State { PLAYING, WON, LOST };

    MinesweeperGame();
    void reset();
    void moveCursor(int dx, int dy);
    void reveal();
    void toggleFlag();

    int   getCursorX()              const { return cx; }
    int   getCursorY()              const { return cy; }
    bool  isMine(int r, int c)      const { return mine[r][c]; }
    bool  isRevealed(int r, int c)  const { return revealed[r][c]; }
    bool  isFlagged(int r, int c)   const { return flagged[r][c]; }
    int   getAdj(int r, int c)      const { return adj[r][c]; }
    State getState()                const { return state; }
    int   getFlagsLeft()            const { return MINES - flagCount; }

    const bool (*getRevealed() const)[COLS] { return revealed; }
    const bool (*getFlagged()  const)[COLS] { return flagged; }
    const bool (*getMines()    const)[COLS] { return mine; }
    const int  (*getAdjs()     const)[COLS] { return adj; }

private:
    bool mine[ROWS][COLS];
    bool revealed[ROWS][COLS];
    bool flagged[ROWS][COLS];
    int  adj[ROWS][COLS];
    int  cx, cy;
    int  flagCount, hiddenSafe;
    bool firstReveal;
    State state;

    void placeMines(int avoidR, int avoidC);
    void calcAdj();
    void floodReveal(int r, int c);
};
