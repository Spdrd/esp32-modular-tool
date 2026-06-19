#pragma once
#include <Arduino.h>
#include "../audio/notes.h"

class SpeakerManager {
public:
    SpeakerManager(int pin, int ledcChannel);

    void begin();

    void play(const Song* song, bool loopPlayback = true);
    void pause();
    void resume();
    void stop();
    void togglePause();

    // Llama en cada loop. Retorna true cuando cambia de nota (redibujar).
    bool update();

    bool        isPlaying()    const { return active && !paused; }
    bool        isPaused()     const { return active && paused; }
    bool        isActive()     const { return active; }
    const char* getSongName()  const { return currentSong ? currentSong->name : ""; }
    int         getNoteIdx()   const { return noteIdx; }
    int         getNoteCount() const { return currentSong ? currentSong->count : 0; }

private:
    int pin;
    int chan;

    const Song*   currentSong;
    int           noteIdx;
    bool          active;
    bool          paused;
    bool          looping;
    bool          inGap;
    unsigned long noteEndMs;

    void silence();
    void startNote(int idx);
    int  noteDurationMs(uint8_t beats) const;
};
