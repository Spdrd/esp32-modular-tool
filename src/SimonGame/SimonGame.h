#pragma once
#include <Arduino.h>

class SimonGame {
public:
    enum State { SHOWING, WAITING, WIN_STEP, LOSE };

    static const int MAX_SEQ = 20;

    SimonGame();

    void reset();
    bool update();
    bool pressButton(int btn); // 0=UP, 1=DOWN, 2=LEFT, 3=RIGHT

    State getState()     const { return state; }
    int   getHighlight() const { return highlight; } // -1=none, 0-3=button
    int   getScore()     const { return score; }
    bool  isFlashOn()    const { return flashOn; }

private:
    int  sequence[MAX_SEQ];
    int  seqLen;
    int  showIdx;
    int  inputIdx;
    int  score;
    State state;

    int  highlight;
    bool flashOn;
    bool dirty;

    unsigned long nextMs;
    unsigned long flashMs;

    void addStep();
    void markDirty();
};
