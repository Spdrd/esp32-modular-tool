#pragma once
#include <Arduino.h>

// Codigos USB HID. Son los valores que van dentro del reporte HID, no ASCII.
//
// El descriptor declara el rango COMPLETO (0x00-0xFF en teclado, 0x000-0x3FF en
// consumer), asi que cualquier codigo de estas paginas se puede enviar sin
// tocar el descriptor. Esto importa: cambiar el descriptor obliga a re-emparejar
// todos los hosts, porque cachean las capacidades del primer emparejamiento.
// Las constantes de aqui son un atajo legible, no un limite.

// =====================================================
// Teclado / Keypad (Usage Page 0x07)
// =====================================================

// --- Modificadores (bitmask del byte 0 del reporte) ---
#define HID_MOD_NONE     0x00
#define HID_MOD_LCTRL    0x01
#define HID_MOD_LSHIFT   0x02
#define HID_MOD_LALT     0x04
#define HID_MOD_LGUI     0x08
#define HID_MOD_RCTRL    0x10
#define HID_MOD_RSHIFT   0x20
#define HID_MOD_RALT     0x40
#define HID_MOD_RGUI     0x80

// --- Letras ---
#define HID_KEY_A        0x04
#define HID_KEY_B        0x05
#define HID_KEY_C        0x06
#define HID_KEY_D        0x07
#define HID_KEY_E        0x08
#define HID_KEY_F        0x09
#define HID_KEY_G        0x0A
#define HID_KEY_H        0x0B
#define HID_KEY_I        0x0C
#define HID_KEY_J        0x0D
#define HID_KEY_K        0x0E
#define HID_KEY_L        0x0F
#define HID_KEY_M        0x10
#define HID_KEY_N        0x11
#define HID_KEY_O        0x12
#define HID_KEY_P        0x13
#define HID_KEY_Q        0x14
#define HID_KEY_R        0x15
#define HID_KEY_S        0x16
#define HID_KEY_T        0x17
#define HID_KEY_U        0x18
#define HID_KEY_V        0x19
#define HID_KEY_W        0x1A
#define HID_KEY_X        0x1B
#define HID_KEY_Y        0x1C
#define HID_KEY_Z        0x1D

// --- Numeros (fila superior) ---
#define HID_KEY_1        0x1E
#define HID_KEY_2        0x1F
#define HID_KEY_3        0x20
#define HID_KEY_4        0x21
#define HID_KEY_5        0x22
#define HID_KEY_6        0x23
#define HID_KEY_7        0x24
#define HID_KEY_8        0x25
#define HID_KEY_9        0x26
#define HID_KEY_0        0x27

// --- Control basico ---
#define HID_KEY_ENTER     0x28
#define HID_KEY_ESC       0x29
#define HID_KEY_BACKSPACE 0x2A
#define HID_KEY_TAB       0x2B
#define HID_KEY_SPACE     0x2C

// --- Puntuacion ---
#define HID_KEY_MINUS      0x2D  // - _
#define HID_KEY_EQUAL      0x2E  // = +
#define HID_KEY_LBRACKET   0x2F  // [ {
#define HID_KEY_RBRACKET   0x30  // ] }
#define HID_KEY_BACKSLASH  0x31  // \ |
#define HID_KEY_SEMICOLON  0x33  // ; :
#define HID_KEY_QUOTE      0x34  // ' "
#define HID_KEY_GRAVE      0x35  // ` ~
#define HID_KEY_COMMA      0x36  // , <
#define HID_KEY_PERIOD     0x37  // . >
#define HID_KEY_SLASH      0x38  // / ?
#define HID_KEY_CAPSLOCK   0x39

// --- Funcion ---
#define HID_KEY_F1       0x3A
#define HID_KEY_F2       0x3B
#define HID_KEY_F3       0x3C
#define HID_KEY_F4       0x3D
#define HID_KEY_F5       0x3E
#define HID_KEY_F6       0x3F
#define HID_KEY_F7       0x40
#define HID_KEY_F8       0x41
#define HID_KEY_F9       0x42
#define HID_KEY_F10      0x43
#define HID_KEY_F11      0x44
#define HID_KEY_F12      0x45

// --- Navegacion y sistema ---
#define HID_KEY_PRINTSCREEN 0x46
#define HID_KEY_SCROLLLOCK  0x47
#define HID_KEY_PAUSE       0x48
#define HID_KEY_INSERT      0x49
#define HID_KEY_HOME        0x4A
#define HID_KEY_PAGEUP      0x4B
#define HID_KEY_DELETE      0x4C
#define HID_KEY_END         0x4D
#define HID_KEY_PAGEDOWN    0x4E

// --- Flechas ---
#define HID_KEY_RIGHT    0x4F
#define HID_KEY_LEFT     0x50
#define HID_KEY_DOWN     0x51
#define HID_KEY_UP       0x52

// --- Teclado numerico ---
#define HID_KEY_NUMLOCK  0x53
#define HID_KEY_KP_DIV   0x54
#define HID_KEY_KP_MUL   0x55
#define HID_KEY_KP_SUB   0x56
#define HID_KEY_KP_ADD   0x57
#define HID_KEY_KP_ENTER 0x58
#define HID_KEY_KP_1     0x59
#define HID_KEY_KP_2     0x5A
#define HID_KEY_KP_3     0x5B
#define HID_KEY_KP_4     0x5C
#define HID_KEY_KP_5     0x5D
#define HID_KEY_KP_6     0x5E
#define HID_KEY_KP_7     0x5F
#define HID_KEY_KP_8     0x60
#define HID_KEY_KP_9     0x61
#define HID_KEY_KP_0     0x62
#define HID_KEY_KP_DOT   0x63

#define HID_KEY_APPLICATION 0x65  // tecla de menu contextual

// --- Funcion extendida ---
#define HID_KEY_F13      0x68
#define HID_KEY_F14      0x69
#define HID_KEY_F15      0x6A
#define HID_KEY_F16      0x6B
#define HID_KEY_F17      0x6C
#define HID_KEY_F18      0x6D
#define HID_KEY_F19      0x6E
#define HID_KEY_F20      0x6F
#define HID_KEY_F21      0x70
#define HID_KEY_F22      0x71
#define HID_KEY_F23      0x72
#define HID_KEY_F24      0x73

// =====================================================
// Consumer Control (Usage Page 0x0C)
// Son codigos de uso de 16 bits, NO bitmasks: se envia uno por reporte.
// =====================================================

// --- Reproduccion ---
#define HID_CC_PLAY_PAUSE   0x00CD
#define HID_CC_PLAY         0x00B0
#define HID_CC_PAUSE        0x00B1
#define HID_CC_STOP         0x00B7
#define HID_CC_NEXT         0x00B5
#define HID_CC_PREV         0x00B6
#define HID_CC_FAST_FORWARD 0x00B3
#define HID_CC_REWIND       0x00B4

// --- Volumen ---
#define HID_CC_MUTE         0x00E2
#define HID_CC_VOL_UP       0x00E9
#define HID_CC_VOL_DOWN     0x00EA

// --- Pantalla ---
#define HID_CC_BRIGHT_UP    0x006F
#define HID_CC_BRIGHT_DOWN  0x0070

// =====================================================
// Mouse: mascara de botones del reporte ID 3
// =====================================================
#define HID_MOUSE_LEFT      0x01
#define HID_MOUSE_RIGHT     0x02
#define HID_MOUSE_MIDDLE    0x04
#define HID_MOUSE_BACK      0x08
#define HID_MOUSE_FORWARD   0x10

// --- Aplicaciones / navegador ---
#define HID_CC_CALCULATOR   0x0192
#define HID_CC_EMAIL        0x018A
#define HID_CC_EXPLORER     0x0194
#define HID_CC_SEARCH       0x0221
#define HID_CC_HOME         0x0223
#define HID_CC_BACK         0x0224
#define HID_CC_FORWARD      0x0225
#define HID_CC_REFRESH      0x0227
#define HID_CC_BOOKMARKS    0x022A
