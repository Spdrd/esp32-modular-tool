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

// -------------------------------------------------------
// noteDurationMs
// beats=4 → quarter note = 60000/bpm ms
// -------------------------------------------------------
int SpeakerManager::noteDurationMs(uint8_t beats) const {
    if (!currentSong || currentSong->bpm <= 0) return 250;
    return (60000 / currentSong->bpm) * beats / 4;
}

void SpeakerManager::startNote(int idx) {
    const MusNote& n = currentSong->notes[idx];
    int dur = noteDurationMs(n.beats);
    // Deja 15ms de silencio al final para articulación
    int playMs = (dur > 20) ? dur - 15 : dur;
    ledcWriteTone(chan, n.freq);
    noteEndMs = millis() + playMs;
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
        // Nota terminó → silencio breve (gap de articulación)
        silence();
        noteEndMs = now + 15;
        inGap     = true;
        return false;
    }

    // Gap terminó → siguiente nota
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
