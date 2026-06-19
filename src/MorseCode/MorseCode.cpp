#include "MorseCode.h"
#include <string.h>

// ---------------------------------------------------------------------------
// Morse table: each entry is { morse pattern, decoded character }
// ---------------------------------------------------------------------------
struct MorseEntry {
    const char* code;
    char        ch;
};

static const MorseEntry MORSE_TABLE[] = {
    // Letters A-Z
    { ".-",   'A' },
    { "-...", 'B' },
    { "-.-.", 'C' },
    { "-..",  'D' },
    { ".",    'E' },
    { "..-.", 'F' },
    { "--.",  'G' },
    { "....", 'H' },
    { "..",   'I' },
    { ".---", 'J' },
    { "-.-",  'K' },
    { ".-..", 'L' },
    { "--",   'M' },
    { "-.",   'N' },
    { "---",  'O' },
    { ".--.", 'P' },
    { "--.-", 'Q' },
    { ".-.",  'R' },
    { "...",  'S' },
    { "-",    'T' },
    { "..-",  'U' },
    { "...-", 'V' },
    { ".--",  'W' },
    { "-..-", 'X' },
    { "-.--", 'Y' },
    { "--..", 'Z' },
    // Digits 0-9
    { "-----", '0' },
    { ".----", '1' },
    { "..---", '2' },
    { "...--", '3' },
    { "....-", '4' },
    { ".....", '5' },
    { "-....", '6' },
    { "--...", '7' },
    { "---..", '8' },
    { "----.", '9' },
};

static const int MORSE_TABLE_SIZE = (int)(sizeof(MORSE_TABLE) / sizeof(MORSE_TABLE[0]));

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
MorseCode::MorseCode() {
    reset();
}

// ---------------------------------------------------------------------------
// reset
// ---------------------------------------------------------------------------
void MorseCode::reset() {
    pattern[0]  = '\0';
    output[0]   = '\0';
    patternLen  = 0;
    lastDecoded = ' ';
}

// ---------------------------------------------------------------------------
// addDot
// ---------------------------------------------------------------------------
void MorseCode::addDot() {
    if (patternLen < MAX_PATTERN) {
        pattern[patternLen] = '.';
        patternLen++;
        pattern[patternLen] = '\0';
    }
}

// ---------------------------------------------------------------------------
// addDash
// ---------------------------------------------------------------------------
void MorseCode::addDash() {
    if (patternLen < MAX_PATTERN) {
        pattern[patternLen] = '-';
        patternLen++;
        pattern[patternLen] = '\0';
    }
}

// ---------------------------------------------------------------------------
// addSpace
// ---------------------------------------------------------------------------
void MorseCode::addSpace() {
    int outLen = (int)strlen(output);
    if (outLen < MAX_OUTPUT) {
        output[outLen]     = ' ';
        output[outLen + 1] = '\0';
    }
    lastDecoded = ' ';
}

// ---------------------------------------------------------------------------
// deleteLast
// ---------------------------------------------------------------------------
void MorseCode::deleteLast() {
    if (patternLen > 0) {
        patternLen--;
        pattern[patternLen] = '\0';
    }
}

// ---------------------------------------------------------------------------
// clearPattern
// ---------------------------------------------------------------------------
void MorseCode::clearPattern() {
    pattern[0]  = '\0';
    patternLen  = 0;
    lastDecoded = ' ';
}

// ---------------------------------------------------------------------------
// clearAll
// ---------------------------------------------------------------------------
void MorseCode::clearAll() {
    reset();
}

// ---------------------------------------------------------------------------
// confirmChar
// ---------------------------------------------------------------------------
void MorseCode::confirmChar() {
    int outLen = (int)strlen(output);

    if (patternLen == 0) {
        // Empty pattern → append a space separator
        if (outLen < MAX_OUTPUT) {
            output[outLen]     = ' ';
            output[outLen + 1] = '\0';
        }
        lastDecoded = ' ';
    } else {
        char decoded = lookupMorse(pattern);
        lastDecoded  = decoded;

        if (outLen < MAX_OUTPUT) {
            output[outLen]     = decoded;
            output[outLen + 1] = '\0';
        }

        // Clear pattern after decoding
        pattern[0] = '\0';
        patternLen = 0;
    }
}

// ---------------------------------------------------------------------------
// lookupMorse
// ---------------------------------------------------------------------------
char MorseCode::lookupMorse(const char* pat) const {
    for (int i = 0; i < MORSE_TABLE_SIZE; i++) {
        if (strcmp(MORSE_TABLE[i].code, pat) == 0) {
            return MORSE_TABLE[i].ch;
        }
    }
    return '?';
}
