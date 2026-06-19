#pragma once
#include <Arduino.h>

class MorseCode {
public:
    static const int MAX_PATTERN = 8;
    static const int MAX_OUTPUT  = 48;

    MorseCode();
    void reset();

    void addDot();
    void addDash();
    void addSpace();      // agrega espacio de palabra al output directamente
    void deleteLast();
    void confirmChar();   // decodifica patrón actual → agrega a output, limpia patrón
    void clearPattern();
    void clearAll();

    const char* getPattern()     const { return pattern; }
    const char* getOutput()      const { return output; }
    int         getPatternLen()  const { return patternLen; }
    char        getLastDecoded() const { return lastDecoded; }

private:
    char pattern[MAX_PATTERN + 1];
    char output[MAX_OUTPUT + 1];
    int  patternLen;
    char lastDecoded;

    char lookupMorse(const char* pat) const;
};
