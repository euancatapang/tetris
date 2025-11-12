#include <stdint.h>
#include <windows.h>

// TETROMINO SHAPES — 4 rotations each, 4x4 grids
// Order: I, O, T, S, Z, J, L

const int8_t _tetrominoes[7][4][4][4] = {
    // I
    {
        { {0,0,0,0},
          {1,1,1,1},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,0,1,0},
          {0,0,1,0},
          {0,0,1,0},
          {0,0,1,0} },

        { {0,0,0,0},
          {1,1,1,1},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,0,1,0},
          {0,0,1,0},
          {0,0,1,0},
          {0,0,1,0} }
    },

    // O
    {
        { {0,1,1,0},
          {0,1,1,0},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,1,1,0},
          {0,1,1,0},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,1,1,0},
          {0,1,1,0},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,1,1,0},
          {0,1,1,0},
          {0,0,0,0},
          {0,0,0,0} }
    },

    // T
    {
        { {0,1,0,0},
          {1,1,1,0},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,1,0,0},
          {0,1,1,0},
          {0,1,0,0},
          {0,0,0,0} },

        { {0,0,0,0},
          {1,1,1,0},
          {0,1,0,0},
          {0,0,0,0} },

        { {0,1,0,0},
          {1,1,0,0},
          {0,1,0,0},
          {0,0,0,0} }
    },

    // S
    {
        { {0,1,1,0},
          {1,1,0,0},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,1,0,0},
          {0,1,1,0},
          {0,0,1,0},
          {0,0,0,0} },

        { {0,1,1,0},
          {1,1,0,0},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,1,0,0},
          {0,1,1,0},
          {0,0,1,0},
          {0,0,0,0} }
    },

    // Z
    {
        { {1,1,0,0},
          {0,1,1,0},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,0,1,0},
          {0,1,1,0},
          {0,1,0,0},
          {0,0,0,0} },

        { {1,1,0,0},
          {0,1,1,0},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,0,1,0},
          {0,1,1,0},
          {0,1,0,0},
          {0,0,0,0} }
    },

    // J
    {
        { {1,0,0,0},
          {1,1,1,0},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,1,1,0},
          {0,1,0,0},
          {0,1,0,0},
          {0,0,0,0} },

        { {0,0,0,0},
          {1,1,1,0},
          {0,0,1,0},
          {0,0,0,0} },

        { {0,1,0,0},
          {0,1,0,0},
          {1,1,0,0},
          {0,0,0,0} }
    },

    // L
    {
        { {0,0,1,0},
          {1,1,1,0},
          {0,0,0,0},
          {0,0,0,0} },

        { {0,1,0,0},
          {0,1,0,0},
          {0,1,1,0},
          {0,0,0,0} },

        { {0,0,0,0},
          {1,1,1,0},
          {1,0,0,0},
          {0,0,0,0} },

        { {1,1,0,0},
          {0,1,0,0},
          {0,1,0,0},
          {0,0,0,0} }
    }
};

const WORD _colors[7][2] = {
    // I
    {
        FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY
    },

    // O
    {
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY
    },

    // T
    {
        FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY
    },

    // S
    {
        FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        FOREGROUND_GREEN | FOREGROUND_INTENSITY
    },

    // Z
    {
        FOREGROUND_RED | FOREGROUND_INTENSITY,
        FOREGROUND_RED | FOREGROUND_INTENSITY
    },

    // J
    {
        FOREGROUND_RED | FOREGROUND_GREEN,
        FOREGROUND_RED | FOREGROUND_GREEN
    },

    // L
    {
        FOREGROUND_BLUE,
        FOREGROUND_BLUE
    }
};