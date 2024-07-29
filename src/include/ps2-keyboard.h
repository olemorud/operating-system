#pragma once 

#include "str.h"

/* commands that the system sends to the keyboard */
enum ps2_cmd {
    /* Set LEDs */
    PS2CMD_SET_LEDS                   = 0xED,

    /* Echo (for diagnostic purposes, and useful for device removal detection) */
    PS2CMD_ECHO                       = 0xEE,

    /* Get/set current scan code set */
    PS2CMD_GET_SCAN                   = 0xF0,

    /* Identify keyboard */
    PS2CMD_IDENTIFY_KEYBOARD          = 0xF2,

    /* Set typematic rate and delay */
    PS2CMD_SET_TYPEMATIC_RATE         = 0xF3,

    /* Enable scanning (keyboard will send scan codes) */
    PS2CMD_ENABLE_SCANNING            = 0xF4,

    /* Disable scanning (keyboard won't send scan codes)*/
    /* Note: May also restore default parameters */
    PS2CMD_DISABLE_SCANNING           = 0xF5,

    /* Set default parameters */
    PS2CMD_SET_DEFAULT_PARAMETERS     = 0xF6,

    /* Set all keys to typematic/autorepeat only (scancode set 3 only) */
    PS2CMD_SET_ALL_KEYS_TO_TA         = 0xF7,

    /* Set all keys to make/release (scancode set 3 only) */
    PS2CMD_SET_ALL_KEYS_TO_MR         = 0xF8,

    /* Set all keys to make only (scancode set 3 only) */
    PS2CMD_SET_ALL_KEYS_TO_M          = 0xF9,

    /* Set all keys to typematic/autorepeat/make/release (scancode set 3 only) */
    PS2CMD_SET_ALL_KEYS_TO_TAMR       = 0xFA,

    /* Set specific key to typematic/autorepeat only (scancode set 3 only) */
    PS2CMD_SET_KEY_TO_TA              = 0xFB,

    /* Set specific key to make/release (scancode set 3 only) */
    PS2CMD_SET_KEY_TO_MR              = 0xFC,

    /* Set specific key to make only (scancode set 3 only) */
    PS2CMD_SET_KEY_TO_M               = 0xFD,

    /* Resend last byte */
    PS2CMD_RESEND_LAST_BYTE           = 0xFE,

    /* Reset and start self-test */
    PS2CMD_RESEND_RESET_AND_SELF_TEST = 0xFF,

    PS2CMD_COUNT,
};

/* commands that the keyboard sends to the system */
enum ps2_response {

    /* Self test passed (sent after "0xFF (reset)" command or keyboard power up) */
    PS2RESPONSE_SELF_TEST_PASSED   = 0xAA,

    /* Response to "0xEE (echo)" command */
    PS2RESPONSE_ECHO_RESPONSE      = 0xEE,

    /* Command acknowledged (ACK) */
    PS2RESPONSE_ACK                = 0xFA,

    /* Self test failed (sent after "0xFF (reset)" command or keyboard power up) */
    PS2RESPONSE_SELF_TEST_FAILED_1 = 0xFC,
    PS2RESPONSE_SELF_TEST_FAILED_2 = 0xFD,

    /* Resend (keyboard wants controller to repeat last command it sent) */
    PS2RESPONSE_RESEND             = 0xFE,

    /* Key detection error or internal buffer overrun */
    PS2RESPONSE_ERROR_1            = 0x00,
    PS2RESPONSE_ERROR_2            = 0xFF,

    PS2RESPONSE_COUNT,
};

extern const struct str ps2_cmd_str[PS2CMD_COUNT];

extern const struct str ps2_response_str[PS2RESPONSE_COUNT];

