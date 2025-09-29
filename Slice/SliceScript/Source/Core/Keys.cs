using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public enum Keys
    {
        // KEY_ = 0x,

        // Misc Keys
        KEY_BACKSPACE = 0x08,
        KEY_TAB = 0x09,
        KEY_ENTER = 0x0D,
        KEY_CTRL = 0x11,
        KEY_CAPSLOCK = 0x14,
        KEY_ESC = 0x100,
        KEY_SPACEBAR = 0x20,

        // Arrow Keys
        KEY_ARROW_LEFT = 0x25,
        KEY_ARROW_UP = 0x26,
        KEY_ARROW_RIGHT = 0x27,
        KEY_ARROW_DOWN = 0x28,

        // Misc Keys
        KEY_PRINT_SCREEN = 0x2C,
        KEY_INERT = 0x2D,
        KEY_DELETE = 0x2E,
        KEY_SHIFT_LEFT = 0xA2,
        KEY_SHIFT_RIGHT = 0xA3,
        KEY_ALT_LEFT = 0xA4,
        KEY_ALT_RIGHT = 0xA5,

        // Volume Keys
        KEY_VOLUME_MUTE = 0xAD,
        KEY_VOLUME_DOWN = 0xAE,
        KEY_VOLUME_UP = 0xAF,

        // Misc Keys
        KEY_PLUS = 0xBB,
        KEY_COMMA = 0xBC,
        KEY_DASH = 0xBD,
        KEY_FULLSTOP = 0xBE,
        KEY_QUESTIONMARK = 0xBF,
        KEY_CURLYDASH = 0xC0,
        KEY_CURLYBRACKET_OPEN = 0xDB,
        KEY_BACKDASH = 0xDC,
        KEY_CURLYBRACKET_CLOSE = 0xDD,
        KEY_APOSTROPHE = 0xDE,
        KEY_WINDOWS_KEY = 0x5B,

        // Track Keys
        KEY_TRACK_NEXT = 0xB0,
        KEY_TRACK_PREVIOUS = 0xB1,
        KEY_TRACK_STOP = 0xB2,
        KEY_TRACK_PLAYORPAUSE = 0xB3,

        // Number Keys
        KEY_0 = 0x30,
        KEY_1 = 0x31,
        KEY_2 = 0x32,
        KEY_3 = 0x33,
        KEY_4 = 0x34,
        KEY_5 = 0x35,
        KEY_6 = 0x36,
        KEY_7 = 0x37,
        KEY_8 = 0x38,
        KEY_9 = 0x39,

        // Alphabet Keys
        KEY_A = 0x41,
        KEY_B = 0x42,
        KEY_C = 0x43,
        KEY_D = 0x44,
        KEY_E = 0x45,
        KEY_F = 0x46,
        KEY_G = 0x47,
        KEY_H = 0x48,
        KEY_I = 0x49,
        KEY_J = 0x4A,
        KEY_K = 0x4B,
        KEY_L = 0x4C,
        KEY_M = 0x4D,
        KEY_N = 0x4E,
        KEY_O = 0x4F,
        KEY_P = 0x50,
        KEY_Q = 0x51,
        KEY_R = 0x52,
        KEY_S = 0x53,
        KEY_T = 0x54,
        KEY_U = 0x55,
        KEY_V = 0x56,
        KEY_W = 0x57,
        KEY_X = 0x58,
        KEY_Y = 0x59,
        KEY_Z = 0x5A,

        // F Keys
        KEY_F01 = 0x70,
        KEY_F02 = 0x71,
        KEY_F03 = 0x72,
        KEY_F04 = 0x73,
        KEY_F05 = 0x74,
        KEY_F06 = 0x75,
        KEY_F07 = 0x76,
        KEY_F08 = 0x77,
        KEY_F09 = 0x78,
        KEY_F10 = 0x79,
        KEY_F11 = 0x7A,
        KEY_F12 = 0x7B,

        // MAX_KEYBOARD_KEYS,
    };

    public enum MouseButtons
    {
        MOUSE_BUTTON_LEFT = 0,
        MOUSE_BUTTON_RIGHT = 1,
        MOUSE_BUTTON_WHEEL = 2,
        MAX_MOUSE_BUTONS,
    };
}
