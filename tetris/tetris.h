#pragma once

#include <stdint.h>
#include <windows.h>

// BOARD CONSTANTS
#define TETROMINO_CELL_ROWS 20 // DEFAULT 20
#define TETROMINO_CELL_COLS 10 // DEFAULT 10

#define TERMINAL_ROWS (TETROMINO_CELL_ROWS*1) // DEFAULT (TETROMINO_CELL_ROWS*1)
#define TERMINAL_COLS (TETROMINO_CELL_COLS*2) // DEFAULT (TETROMINO_CELL_ROWS*2)

#define BOARD_HEIGHT (TERMINAL_ROWS+2)
#define BOARD_WIDTH (TERMINAL_COLS+2)

// BORDERS
#define BORDER_WIDTH 1

#define NEXT_PIECE 0 
#define CACHE_BOX 11 // Y-axis offset

// SHAPE PREVIEW OFFSET
#define SHAPE_PREVIEW_OFFSET_X (BOARD_WIDTH+7)
#define SHAPE_PREVIEW_OFFSET_Y 2

// CONTROLS OFFSET
#define CONTROLS_OFFSET 1

// INPUT ACTIONS
#define LEFT 1
#define RIGHT 2
#define DOWN 3
#define UP 4
#define CLOCKWISE 5
#define A_CLOCKWISE 6
#define HARD_DOWN 7
#define CACHE_PIECE 8

// ACTION TYPES
#define MOVE 1
#define ROTATE 2
#define SPECIAL 3

// PLACE SHAPE MODES
#define CLEAR 0
#define PLACE_SHAPE 1

#define NEEDS_KICK_FROM_RIGHT -1
#define NEEDS_KICK_FROM_LEFT -2
#define NEEDS_KICK_FROM_TOP -3

// TETROMINOES
#define _I 0
#define _O 1
#define _T 2
#define _S 3
#define _Z 4
#define _J 5
#define _L 6

// DEFAULT SHAPE SPAWN
#define DEFAULT_X (TETROMINO_CELL_COLS/2-1)
#define DEFAULT_Y 0
#define DEFAULT_ROTATION 0

// CELL
#define FILLED_CELL L"██"

#define HALF_CELL L'█'
#define EMPTY_HALF_CELL L' '

// BAG RANDOMIZER
#define BAG_SIZE 7

// HARDCODED TETROMINOES
extern const int8_t _tetrominoes[7][4][4][4];
extern const WORD _colors[7];

// GLOBALS
typedef struct {
    int8_t x;
    int8_t y;
    int8_t shape;
    int8_t rotation;
    int8_t borderedBoardWithoutShape[TETROMINO_CELL_ROWS + 2][TETROMINO_CELL_COLS + 2];

} Shape;

typedef struct {
    uint8_t name;
    uint8_t type;

} Action;

typedef struct {
    uint8_t index;
    uint8_t shapes[BAG_SIZE];
    
} Bag;

typedef struct {
    uint32_t score;
    uint32_t previousScore;
    uint32_t highScore;

} Scores;
