#pragma once

#include <stddef.h>
#include <stdint.h>
#include "str.h"

enum key {
    KEY_RELEASED = 0x80,
    KEY_EXTENDED = 0xE0,

    KEY_ESCAPE               = 0x01,
    KEY_1                    = 0x02,
    KEY_2                    = 0x03,
    KEY_3                    = 0x04,
    KEY_4                    = 0x05,
    KEY_5                    = 0x06,
    KEY_6                    = 0x07,
    KEY_7                    = 0x08,
    KEY_8                    = 0x09,
    KEY_9                    = 0x0A,
    KEY_0                    = 0x0B,
    KEY_MINUS                = 0x0C,
    KEY_EQUALS               = 0x0D,
    KEY_BACKSPACE            = 0x0E,
    KEY_TAB                  = 0x0F,
    KEY_Q                    = 0x10,
    KEY_W                    = 0x11,
    KEY_E                    = 0x12,
    KEY_R                    = 0x13,
    KEY_T                    = 0x14,
    KEY_Y                    = 0x15,
    KEY_U                    = 0x16,
    KEY_I                    = 0x17,
    KEY_O                    = 0x18,
    KEY_P                    = 0x19,
    KEY_SQUARE_BRACKET_LEFT  = 0x1A,
    KEY_SQUARE_BRACKET_RIGHT = 0x1B,
    KEY_ENTER                = 0x1C,
    KEY_LEFT_CONTROL         = 0x1D,
    KEY_A                    = 0x1E,
    KEY_S                    = 0x1F,
    KEY_D                    = 0x20,
    KEY_F                    = 0x21,
    KEY_G                    = 0x22,
    KEY_H                    = 0x23,
    KEY_J                    = 0x24,
    KEY_K                    = 0x25,
    KEY_L                    = 0x26,
    KEY_SEMICOLON            = 0x27,
    KEY_SINGLE_QUOTE         = 0x28,
    KEY_BACKTICK             = 0x29,
    KEY_LEFT_SHIFT           = 0x2A,
    KEY_BACKSLASH            = 0x2B,
    KEY_Z                    = 0x2C,
    KEY_X                    = 0x2D,
    KEY_C                    = 0x2E,
    KEY_V                    = 0x2F,
    KEY_B                    = 0x30,
    KEY_N                    = 0x31,
    KEY_M                    = 0x32,
    KEY_COMMA                = 0x33,
    KEY_DOT                  = 0x34,
    KEY_FORWARD_SLASH        = 0x35,
    KEY_RIGHT_SHIFT          = 0x36,
    KEY_KEYPAD_ASTERISK      = 0x37,
    KEY_LEFT_ALT             = 0x38,
    KEY_SPACE                = 0x39,
    KEY_CAPSLOCK             = 0x3A,
    KEY_F1                   = 0x3B,
    KEY_F2                   = 0x3C,
    KEY_F3                   = 0x3D,
    KEY_F4                   = 0x3E,
    KEY_F5                   = 0x3F,
    KEY_F6                   = 0x40,
    KEY_F7                   = 0x41,
    KEY_F8                   = 0x42,
    KEY_F9                   = 0x43,
    KEY_F10                  = 0x44,
    KEY_NUMBERLOCK           = 0x45,
    KEY_SCROLLLOCK           = 0x46,
    KEY_KEYPAD_7             = 0x47,
    KEY_KEYPAD_8             = 0x48,
    KEY_KEYPAD_9             = 0x49,
    KEY_KEYPAD_MINUS         = 0x4A,
    KEY_KEYPAD_4             = 0x4B,
    KEY_KEYPAD_5             = 0x4C,
    KEY_KEYPAD_6             = 0x4D,
    KEY_KEYPAD_PLUS          = 0x4E,
    KEY_KEYPAD_1             = 0x4F,
    KEY_KEYPAD_2             = 0x50,
    KEY_KEYPAD_3             = 0x51,
    KEY_KEYPAD_0             = 0x52,
    KEY_KEYPAD_DOT           = 0x53,
    KEY_F11                  = 0x57,
    KEY_F12                  = 0x58,
};
extern const struct str key_str[];

 /* keys_default_keymap[k | KEY_MODIFIER_SHIFT] --> shift key value of key `k` */
constexpr size_t KEY_MODIFIER_SHIFT = 0x80;

extern const char keys_default_keymap[];

/* Not supported at the moment */
enum extended_keys : uint8_t {
    KEY_E_MULTIMEDIA_PREV_TRACK   = 0x10,
    KEY_E_MULTIMEDIA_NEXT_TRACK   = 0x19, 
    KEY_E_KEYPAD_ENTER            = 0x1C, 
    KEY_E_RIGHT_CONTROL           = 0x1D, 
    KEY_E_MULTIMEDIA_MUTE         = 0x20, 
    KEY_E_MULTIMEDIA_CALCULATOR   = 0x21, 
    KEY_E_MULTIMEDIA_PLAY         = 0x22, 
    KEY_E_MULTIMEDIA_STOP         = 0x24, 
    KEY_E_MULTIMEDIA_VOLUME_DOWN  = 0x2E, 
    KEY_E_MULTIMEDIA_VOLUME_UP    = 0x30, 
    KEY_E_MULTIMEDIA_WWW_HOME     = 0x32, 
    KEY_E_KEYPAD_FORWARD_SLASH    = 0x35, 
    KEY_E_RIGHT_ALT               = 0x38, 
    KEY_E_HOME                    = 0x47, 
    KEY_E_CURSOR_UP               = 0x48, 
    KEY_E_PAGE_UP                 = 0x49, 
    KEY_E_CURSOR_LEFT             = 0x4B, 
    KEY_E_CURSOR_RIGHT            = 0x4D, 
    KEY_E_END                     = 0x4F, 
    KEY_E_CURSOR_DOWN             = 0x50, 
    KEY_E_PAGE_DOWN               = 0x51, 
    KEY_E_DELETE                  = 0x53, 
    KEY_E_LEFT_GUI                = 0x5B, 
    KEY_E_RIGHT_GUI               = 0x5C, 
    KEY_E_APPS                    = 0x5D, 
    KEY_E_ACPI_SLEEP              = 0x5F, 
    KEY_E_ACPI_WAKE               = 0x63, 
    KEY_E_MULTIMEDIA_WWW_SEARCH   = 0x65, 
    KEY_E_MULTIMEDIA_WWW_REFRESH  = 0x67, 
    KEY_E_MULTIMEDIA_WWW_STOP     = 0x68, 
    KEY_E_MULTIMEDIA_WWW_FORWARD  = 0x69, 
    KEY_E_MULTIMEDIA_MY_COMPUTER  = 0x6B, 
    KEY_E_MULTIMEDIA_EMAIL        = 0x6C, 
    KEY_E_MULTIMEDIA_MEDIA_SELECT = 0x6D, 
    /* TODO: implement these:
       0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5	pause pressed
       0x2A, 0xE0, 0x37	                    print screen pressed
       0xB7, 0xE0, 0xAA	                    print screen released
    */
};

