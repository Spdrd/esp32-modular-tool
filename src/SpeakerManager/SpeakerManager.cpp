#include "SpeakerManager.h"

SpeakerManager::SpeakerManager(int pin, int ledcChannel)
    : pin(pin), chan(ledcChannel),
      currentSong(nullptr), noteIdx(0),
      active(false), paused(false), looping(true), inGap(false), noteEndMs(0)
{}

void SpeakerManager::begin() {
    ledcSetup(chan, 2000, 8);
    ledcAttachPin(pin, chan);
    silence();
}

void SpeakerManager::silence() {
    ledcWriteTone(chan, 0);
}

void SpeakerManager::startNote(int idx) {
    const MusNote& n = currentSong->notes[idx];
    ledcWriteTone(chan, n.freq);
    noteEndMs = millis() + n.durationMs;
    inGap = false;
}

// -------------------------------------------------------
// play
// -------------------------------------------------------
void SpeakerManager::play(const Song* song, bool loopPlayback) {
    stop();
    if (!song || song->count == 0) return;
    currentSong = song;
    noteIdx  = 0;
    active   = true;
    paused   = false;
    looping  = loopPlayback;
    startNote(0);
}

// -------------------------------------------------------
// playTone / stopTone — tono continuo, sin gestión de notas
// -------------------------------------------------------
void SpeakerManager::playTone(uint16_t freq) {
    stop();
    ledcWriteTone(chan, freq);
}

void SpeakerManager::stopTone() {
    silence();
}

// -------------------------------------------------------
// pause / resume / stop / togglePause
// -------------------------------------------------------
void SpeakerManager::pause() {
    if (!active || paused) return;
    paused = true;
    silence();
}

void SpeakerManager::resume() {
    if (!active || !paused) return;
    paused = false;
    startNote(noteIdx);
}

void SpeakerManager::stop() {
    active      = false;
    paused      = false;
    currentSong = nullptr;
    noteIdx     = 0;
    silence();
}

void SpeakerManager::togglePause() {
    if (isPaused()) resume();
    else            pause();
}

// -------------------------------------------------------
// update — llama en cada loop()
// -------------------------------------------------------
bool SpeakerManager::update() {
    if (!active || paused || !currentSong) return false;

    unsigned long now = millis();
    if (now < noteEndMs) return false;

    if (!inGap) {
        silence();
        uint16_t gap = currentSong->notes[noteIdx].gapMs;
        if (gap > 0) {
            noteEndMs = now + gap;
            inGap     = true;
            return false;
        }
    }

    inGap   = false;
    noteIdx++;
    if (noteIdx >= currentSong->count) {
        if (looping) {
            noteIdx = 0;
        } else {
            stop();
            return true;
        }
    }
    startNote(noteIdx);
    return true; // cambio de nota → redibujar
}
