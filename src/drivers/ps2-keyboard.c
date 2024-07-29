
#include "ps2-keyboard.h"

const struct str ps2_cmd_str[PS2CMD_COUNT] = {
    [PS2CMD_SET_LEDS                  ] = str_attach("PS2CMD_SET_LEDS"),
    [PS2CMD_ECHO                      ] = str_attach("PS2CMD_ECHO"),
    [PS2CMD_GET_SCAN                  ] = str_attach("PS2CMD_GET_SCAN"),
    [PS2CMD_IDENTIFY_KEYBOARD         ] = str_attach("PS2CMD_IDENTIFY_KEYBOARD"),
    [PS2CMD_SET_TYPEMATIC_RATE        ] = str_attach("PS2CMD_SET_TYPEMATIC_RATE"),
    [PS2CMD_ENABLE_SCANNING           ] = str_attach("PS2CMD_ENABLE_SCANNING"),
    [PS2CMD_DISABLE_SCANNING          ] = str_attach("PS2CMD_DISABLE_SCANNING"),
    [PS2CMD_SET_DEFAULT_PARAMETERS    ] = str_attach("PS2CMD_SET_DEFAULT_PARAMETERS"),
    [PS2CMD_SET_ALL_KEYS_TO_TA        ] = str_attach("PS2CMD_SET_ALL_KEYS_TO_TA"),
    [PS2CMD_SET_ALL_KEYS_TO_MR        ] = str_attach("PS2CMD_SET_ALL_KEYS_TO_MR"),
    [PS2CMD_SET_ALL_KEYS_TO_M         ] = str_attach("PS2CMD_SET_ALL_KEYS_TO_M"),
    [PS2CMD_SET_ALL_KEYS_TO_TAMR      ] = str_attach("PS2CMD_SET_ALL_KEYS_TO_TAMR"),
    [PS2CMD_SET_KEY_TO_TA             ] = str_attach("PS2CMD_SET_KEY_TO_TA"),
    [PS2CMD_SET_KEY_TO_MR             ] = str_attach("PS2CMD_SET_KEY_TO_MR"),
    [PS2CMD_SET_KEY_TO_M              ] = str_attach("PS2CMD_SET_KEY_TO_M"),
    [PS2CMD_RESEND_LAST_BYTE          ] = str_attach("PS2CMD_RESEND_LAST_BYTE"),
    [PS2CMD_RESEND_RESET_AND_SELF_TEST] = str_attach("PS2CMD_RESEND_RESET_AND_SELF_TEST"),
};

const struct str ps2_response_str[PS2RESPONSE_COUNT] = {
    [PS2RESPONSE_ERROR_1           ] = str_attach("PS2RESPONSE_ERROR_1"),
    [PS2RESPONSE_ERROR_2           ] = str_attach("PS2RESPONSE_ERROR_2"),
    [PS2RESPONSE_SELF_TEST_PASSED  ] = str_attach("PS2RESPONSE_SELF_TEST_PASSED"),
    [PS2RESPONSE_ECHO_RESPONSE     ] = str_attach("PS2RESPONSE_ECHO_RESPONSE"),
    [PS2RESPONSE_ACK               ] = str_attach("PS2RESPONSE_ACK"),
    [PS2RESPONSE_SELF_TEST_FAILED_1] = str_attach("PS2RESPONSE_SELF_TEST_FAILED_1"),
    [PS2RESPONSE_SELF_TEST_FAILED_2] = str_attach("PS2RESPONSE_SELF_TEST_FAILED_2"),
    [PS2RESPONSE_RESEND            ] = str_attach("PS2RESPONSE_RESEND"),
};


